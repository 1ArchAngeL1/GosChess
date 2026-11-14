#include "Server.hh"

#include <iostream>
#include <vector>

GosChess::GameServer::GameServer(unsigned short port)
        : colour_draw_(std::random_device{}()), port_(port) {}

GosChess::GameServer::~GameServer() {
    listener_.close();
}

bool GosChess::GameServer::start() {
    if (listener_.listen(port_) != sf::Socket::Done) {
        std::cerr << "could not listen on port " << port_ << std::endl;
        return false;
    }
    listener_.setBlocking(false);
    selector_.add(listener_);
    tick_clock_.restart();
    running_ = true;
    std::cout << "goschess server listening on port " << port_
              << " (protocol version " << Protocol::VERSION << ")" << std::endl;
    return true;
}

void GosChess::GameServer::run() {
    while (running_) poll();
}

void GosChess::GameServer::poll(unsigned int wait_millis) {
    // The wait has a timeout rather than being indefinite so that the clocks
    // keep running in a game where neither player is sending anything.
    if (selector_.wait(sf::milliseconds(wait_millis))) {
        if (selector_.isReady(listener_)) acceptPending();

        // The ids are collected first because handling one session can erase
        // another - a disconnect ends the opponent's game too - and iterating
        // the map while that happens would walk off the end of it.
        std::vector<session_id_t> readable;
        for (const auto &[id, session]: sessions_) {
            if (session.socket && selector_.isReady(*session.socket)) readable.push_back(id);
        }
        for (session_id_t id: readable) {
            if (sessions_.count(id)) readFrom(id);
        }
    }
    chargeClocks();
}

void GosChess::GameServer::acceptPending() {
    // One accept per readiness notification is not enough when several clients
    // arrive together, so keep accepting until the backlog is empty.
    while (true) {
        auto socket = std::make_unique<sf::TcpSocket>();
        if (listener_.accept(*socket) != sf::Socket::Done) return;
        socket->setBlocking(false);
        const session_id_t id = next_session_id_++;
        selector_.add(*socket);
        sessions_[id].socket = std::move(socket);
        std::cout << "session " << id << " connected" << std::endl;
    }
}

void GosChess::GameServer::readFrom(GosChess::GameServer::session_id_t id) {
    Session &session = sessions_.at(id);
    sf::Packet packet;
    const sf::Socket::Status status = session.socket->receive(packet);
    if (status == sf::Socket::Disconnected || status == sf::Socket::Error) {
        disconnect(id);
        return;
    }
    if (status != sf::Socket::Done) return;

    Protocol::ClientMessage type;
    if (!(packet >> type)) return;
    switch (type) {
        case Protocol::ClientMessage::FIND_GAME:
            handleFindGame(id, packet);
            break;
        case Protocol::ClientMessage::MOVE:
            handleMove(id, packet);
            break;
        case Protocol::ClientMessage::RESIGN:
            handleResign(id);
            break;
        default:
            std::cerr << "session " << id << " sent an unknown message" << std::endl;
            break;
    }
}

void GosChess::GameServer::handleFindGame(GosChess::GameServer::session_id_t id, sf::Packet &packet) {
    Protocol::FindGame request;
    if (!(packet >> request)) return;

    // A version mismatch is told to the client plainly. Letting it through
    // would show up as a game that behaves inexplicably instead of as a
    // connection that was refused.
    if (request.version != Protocol::VERSION) {
        std::cerr << "session " << id << " speaks protocol version " << request.version
                  << ", this server speaks " << Protocol::VERSION << std::endl;
        sf::Packet reply;
        Protocol::GameOver over;
        over.reason = Protocol::GameOverReason::PROTOCOL_ERROR;
        reply << Protocol::ServerMessage::GAME_OVER << over;
        send(id, reply);
        disconnect(id);
        return;
    }

    if (sessions_.at(id).match.has_value()) return;
    forgetWaiting(id);

    const auto waiting = waiting_by_time_limit_.find(request.time_limit_minutes);
    if (waiting != waiting_by_time_limit_.end() && waiting->second != id) {
        const session_id_t opponent = waiting->second;
        waiting_by_time_limit_.erase(waiting);
        pair(opponent, id, request.time_limit_minutes);
        return;
    }

    waiting_by_time_limit_[request.time_limit_minutes] = id;
    sf::Packet reply;
    reply << Protocol::ServerMessage::WAITING;
    send(id, reply);
}

void GosChess::GameServer::pair(GosChess::GameServer::session_id_t first,
                                GosChess::GameServer::session_id_t second,
                                std::uint16_t time_limit_minutes) {
    const match_id_t match_id = next_match_id_++;
    matches_.emplace(match_id, Match(match_id, time_limit_minutes));

    // The draw happens here, on the server, rather than being announced by
    // whichever client happened to be hosting.
    const bool first_is_white = std::uniform_int_distribution<int>(0, 1)(colour_draw_) == 0;
    Players players;
    players.white = first_is_white ? first : second;
    players.black = first_is_white ? second : first;
    match_players_[match_id] = players;

    for (color_t color: {color_t::WHITE, color_t::BLACK}) {
        const session_id_t id = players.of(color);
        Session &session = sessions_.at(id);
        session.match = match_id;
        session.color = color;

        sf::Packet start;
        Protocol::GameStart message;
        message.game_id = match_id;
        message.your_color = color;
        message.time_limit_minutes = time_limit_minutes;
        start << Protocol::ServerMessage::GAME_START << message;
        send(id, start);
    }

    std::cout << "match " << match_id << ": session " << players.white << " as white, session "
              << players.black << " as black" << std::endl;

    broadcastState(match_id);
}

void GosChess::GameServer::handleMove(GosChess::GameServer::session_id_t id, sf::Packet &packet) {
    Protocol::MoveRequest request;
    if (!(packet >> request)) return;

    Session &session = sessions_.at(id);
    if (!session.match.has_value() || !matches_.count(session.match.value())) {
        sf::Packet reply;
        Protocol::Rejected rejected;
        rejected.reason = Protocol::RejectReason::NOT_IN_GAME;
        reply << Protocol::ServerMessage::REJECTED << rejected;
        send(id, reply);
        return;
    }

    const match_id_t match_id = session.match.value();
    Match &match = matches_.at(match_id);
    const Match::MoveResult result = match.applyMove(session.color, request);

    if (!result.accepted) {
        // The refusal carries the real position, so a client that was wrong
        // does not have to ask what the truth is - it snaps back to it.
        std::cerr << "match " << match_id << ": refused a move from session " << id << " ("
                  << Protocol::Describe(result.reject_reason) << ")" << std::endl;
        sf::Packet reply;
        Protocol::Rejected rejected;
        rejected.reason = result.reject_reason;
        rejected.state = match.state();
        reply << Protocol::ServerMessage::REJECTED << rejected;
        send(id, reply);
        return;
    }

    broadcastState(match_id);
    if (match.finished()) concludeMatch(match_id);
}

void GosChess::GameServer::handleResign(GosChess::GameServer::session_id_t id) {
    Session &session = sessions_.at(id);
    if (!session.match.has_value() || !matches_.count(session.match.value())) return;
    const match_id_t match_id = session.match.value();
    matches_.at(match_id).resign(session.color);
    broadcastState(match_id);
    concludeMatch(match_id);
}

void GosChess::GameServer::chargeClocks() {
    // Reading the elapsed time in whole milliseconds throws away the remainder
    // on every pass, and at twenty passes a second that lost roughly two per
    // cent of the clock - about fifteen seconds over a ten minute game, all of
    // it in the mover's favour. The remainder is carried instead.
    charge_remainder_micros_ += tick_clock_.restart().asMicroseconds();
    if (charge_remainder_micros_ < 1000) return;
    const std::uint32_t elapsed = static_cast<std::uint32_t>(charge_remainder_micros_ / 1000);
    charge_remainder_micros_ -= static_cast<std::int64_t>(elapsed) * 1000;
    std::vector<match_id_t> flagged;
    for (auto &[match_id, match]: matches_) {
        if (match.chargeTime(elapsed)) flagged.push_back(match_id);
    }
    for (match_id_t match_id: flagged) {
        broadcastState(match_id);
        concludeMatch(match_id);
    }
}

void GosChess::GameServer::broadcastState(GosChess::GameServer::match_id_t match_id) {
    const Match &match = matches_.at(match_id);
    const Players &players = match_players_.at(match_id);
    sf::Packet packet;
    packet << Protocol::ServerMessage::GAME_STATE << match.state();
    for (color_t color: {color_t::WHITE, color_t::BLACK}) {
        sf::Packet copy = packet;
        send(players.of(color), copy);
    }
}

void GosChess::GameServer::concludeMatch(GosChess::GameServer::match_id_t match_id) {
    const auto found = matches_.find(match_id);
    if (found == matches_.end()) return;
    const Players players = match_players_.at(match_id);
    const Protocol::GameOver over = found->second.result();

    std::cout << "match " << match_id << " over: " << Protocol::Describe(over.outcome)
              << " by " << Protocol::Describe(over.reason) << std::endl;

    for (color_t color: {color_t::WHITE, color_t::BLACK}) {
        sf::Packet packet;
        packet << Protocol::ServerMessage::GAME_OVER << over;
        send(players.of(color), packet);
        const auto session = sessions_.find(players.of(color));
        // The connection stays open so the same client can ask for another
        // game; only its tie to this match is cleared.
        if (session != sessions_.end()) session->second.match.reset();
    }

    matches_.erase(match_id);
    match_players_.erase(match_id);
}

void GosChess::GameServer::send(GosChess::GameServer::session_id_t id, sf::Packet &packet) {
    const auto found = sessions_.find(id);
    if (found == sessions_.end() || !found->second.socket) return;
    sf::Socket::Status status;
    // The socket is non-blocking, so a large packet can go out in pieces; send
    // keeps being called with the same packet until SFML has taken all of it.
    do {
        status = found->second.socket->send(packet);
    } while (status == sf::Socket::Partial);
    if (status == sf::Socket::Disconnected || status == sf::Socket::Error) disconnect(id);
}

void GosChess::GameServer::disconnect(GosChess::GameServer::session_id_t id) {
    const auto found = sessions_.find(id);
    if (found == sessions_.end()) return;
    Session session = std::move(found->second);
    sessions_.erase(found);
    forgetWaiting(id);
    if (session.socket) {
        selector_.remove(*session.socket);
        session.socket->disconnect();
    }
    std::cout << "session " << id << " disconnected" << std::endl;

    // Leaving mid-game is a loss, and the opponent is told at once rather than
    // being left staring at a board that will never change.
    if (!session.match.has_value()) return;
    const match_id_t match_id = session.match.value();
    if (!matches_.count(match_id)) return;
    matches_.at(match_id).abandon(session.color);
    // The same two message ending as any other finish, so the client has one
    // code path rather than a special case for an opponent who vanished.
    broadcastState(match_id);
    concludeMatch(match_id);
}

void GosChess::GameServer::forgetWaiting(GosChess::GameServer::session_id_t id) {
    for (auto it = waiting_by_time_limit_.begin(); it != waiting_by_time_limit_.end();) {
        it = it->second == id ? waiting_by_time_limit_.erase(it) : std::next(it);
    }
}
