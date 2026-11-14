#include "ServerConnection.hh"

#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>

#include <SFML/Network/IpAddress.hpp>

#include "../Exec/GamePlay/GamePlayFunctional.hh"
#include "../Game/ClientGame.hh"

namespace {

    sf::TcpSocket connection;

    // Written by the connect thread and read by the draw thread, so it is
    // atomic. The socket itself is only ever touched by the draw thread once
    // the state has left CONNECTING, which is the whole reason this exists.
    std::atomic<GosChess::LinkState> link_state{GosChess::LinkState::OFFLINE};

    std::mutex error_lock;
    std::string link_error;

    void SetError(const std::string &message) {
        std::lock_guard<std::mutex> lock(error_lock);
        link_error = message;
    }

    void Send(sf::Packet &packet) {
        if (link_state == GosChess::LinkState::OFFLINE ||
            link_state == GosChess::LinkState::CONNECTING ||
            link_state == GosChess::LinkState::FAILED) {
            return;
        }
        sf::Socket::Status status;
        do {
            status = connection.send(packet);
        } while (status == sf::Socket::Partial);
    }

    void ConnectAndQueue(std::string address, unsigned short port, std::uint16_t time_limit_minutes) {
        connection.setBlocking(true);
        if (connection.connect(sf::IpAddress(address), port, sf::seconds(5)) != sf::Socket::Done) {
            SetError("could not reach the server at " + address + ":" + std::to_string(port));
            link_state = GosChess::LinkState::FAILED;
            return;
        }

        GosChess::Protocol::FindGame request;
        request.time_limit_minutes = time_limit_minutes;
        sf::Packet packet;
        packet << GosChess::Protocol::ClientMessage::FIND_GAME << request;
        if (connection.send(packet) != sf::Socket::Done) {
            SetError("lost the server while asking for a game");
            link_state = GosChess::LinkState::FAILED;
            return;
        }

        // Non-blocking from here on: the draw thread polls it every frame and
        // must never be left waiting on the network.
        connection.setBlocking(false);
        link_state = GosChess::LinkState::QUEUED;
    }

    // Translates the server's verdict into the result this player sees.
    GosChess::GameResult ResultFor(GosChess::Protocol::GameOutcome outcome) {
        switch (outcome) {
            case GosChess::Protocol::GameOutcome::DRAW:
                return GosChess::GameResult::DRAW;
            case GosChess::Protocol::GameOutcome::WHITE_WINS:
                return GosChess::player_color == GosChess::color_t::WHITE ? GosChess::GameResult::WON
                                                                         : GosChess::GameResult::LOST;
            case GosChess::Protocol::GameOutcome::BLACK_WINS:
                return GosChess::player_color == GosChess::color_t::BLACK ? GosChess::GameResult::WON
                                                                         : GosChess::GameResult::LOST;
        }
        return GosChess::GameResult::DRAW;
    }

    void HandleGameStart(sf::Packet &packet) {
        GosChess::Protocol::GameStart message;
        if (!(packet >> message)) return;
        GosChess::SetPlayerColor(message.your_color);
        GosChess::time_limit_minutes = message.time_limit_minutes;
        link_state = GosChess::LinkState::PLAYING;
        // Drops the menu and lets main() enter the game loop.
        GosChess::game_mode = GosChess::GameMode::MULTI_PLAYER;
        GosChess::menu_active_flag = false;
        std::cout << "game " << message.game_id << ": playing "
                  << (message.your_color == GosChess::color_t::WHITE ? "white" : "black") << std::endl;
    }

    void HandleGameState(sf::Packet &packet) {
        GosChess::Protocol::GameStateUpdate update;
        if (!(packet >> update)) return;
        GosChess::AdoptServerState(update);
    }

    void HandleRejection(sf::Packet &packet) {
        GosChess::Protocol::Rejected rejected;
        if (!(packet >> rejected)) return;
        // Our optimistic move was wrong, so the position we drew is wrong too.
        // Snapping to what came back is the whole repair.
        std::cerr << "the server refused the move: "
                  << GosChess::Protocol::Describe(rejected.reason) << std::endl;
        GosChess::AdoptServerState(rejected.state);
    }

    void HandleGameOver(sf::Packet &packet) {
        GosChess::Protocol::GameOver message;
        if (!(packet >> message)) return;
        std::cout << "game over: " << GosChess::Protocol::Describe(message.outcome)
                  << " by " << GosChess::Protocol::Describe(message.reason) << std::endl;
        if (message.reason == GosChess::Protocol::GameOverReason::PROTOCOL_ERROR) {
            SetError("this client and that server speak different protocol versions");
            link_state = GosChess::LinkState::FAILED;
            return;
        }
        GosChess::game_result = ResultFor(message.outcome);
        GosChess::SetGameFlagFinished();
        link_state = GosChess::LinkState::OFFLINE;
    }

}

char GosChess::server_address[64] = "127.0.0.1";

int GosChess::server_port = GosChess::Protocol::DEFAULT_PORT;

void GosChess::JoinServer(const std::string &address, unsigned short port,
                          std::uint16_t time_limit_minutes) {
    if (link_state == LinkState::CONNECTING) return;
    SetError({});
    link_state = LinkState::CONNECTING;
    std::thread connector(ConnectAndQueue, address, port, time_limit_minutes);
    connector.detach();
}

void GosChess::LeaveServer() {
    if (link_state == LinkState::CONNECTING) return;
    connection.disconnect();
    link_state = LinkState::OFFLINE;
}

GosChess::LinkState GosChess::GetLinkState() {
    return link_state;
}

const std::string &GosChess::GetLinkError() {
    std::lock_guard<std::mutex> lock(error_lock);
    return link_error;
}

void GosChess::SendMove(GosChess::action_t move) {
    Protocol::MoveRequest request;
    // Named for the position the server last confirmed, not for the one we
    // have optimistically drawn.
    request.ply = confirmed_ply;
    request.move_from = move.move_from;
    request.move_to = move.move_to;
    sf::Packet packet;
    packet << Protocol::ClientMessage::MOVE << request;
    Send(packet);
}

void GosChess::SendResign() {
    sf::Packet packet;
    packet << Protocol::ClientMessage::RESIGN;
    Send(packet);
}

void GosChess::ProcessServerMessages() {
    const LinkState state = link_state;
    if (state == LinkState::OFFLINE || state == LinkState::CONNECTING || state == LinkState::FAILED) return;

    // Everything pending is drained each frame, because a single move can be
    // answered with a position and a result in the same breath.
    while (true) {
        sf::Packet packet;
        const sf::Socket::Status status = connection.receive(packet);
        if (status == sf::Socket::Disconnected || status == sf::Socket::Error) {
            SetError("the connection to the server was lost");
            link_state = LinkState::FAILED;
            if (GosChess::game_mode == GosChess::GameMode::MULTI_PLAYER) {
                GosChess::SetGameFlagFinished();
            }
            return;
        }
        if (status != sf::Socket::Done) return;

        Protocol::ServerMessage type;
        if (!(packet >> type)) return;
        switch (type) {
            case Protocol::ServerMessage::WAITING:
                link_state = LinkState::QUEUED;
                break;
            case Protocol::ServerMessage::GAME_START:
                HandleGameStart(packet);
                break;
            case Protocol::ServerMessage::GAME_STATE:
                HandleGameState(packet);
                break;
            case Protocol::ServerMessage::REJECTED:
                HandleRejection(packet);
                break;
            case Protocol::ServerMessage::GAME_OVER:
                HandleGameOver(packet);
                break;
            default:
                std::cerr << "the server sent a message this client does not know" << std::endl;
                break;
        }
    }
}
