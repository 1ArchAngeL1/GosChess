// Drives the real server over real sockets.
//
// server_selftest proves the rules and the clocks; this proves the plumbing
// around them: that two clients get paired, that a move by one reaches the
// other, that an illegal move comes back refused with the true position
// attached, and that walking out of a game does not leave the opponent
// staring at a board that will never change.
//
// The server is stepped by hand rather than run on a thread, so the test is
// deterministic and cannot hang waiting on a race.

#include <cstdio>
#include <memory>
#include <optional>
#include <string>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include "Server.hh"

namespace {

    using namespace GosChess;
    using namespace GosChess::Protocol;

    int failures = 0;

    void Check(bool condition, const std::string &what) {
        if (condition) return;
        std::printf("  FAIL  %s\n", what.c_str());
        failures++;
    }

    constexpr std::int8_t Sq(char file, int rank) {
        return static_cast<std::int8_t>((rank - 1) * board_t::ROW_LENGTH + (file - 'a'));
    }

    // One test client: a socket and the little bit of state a real client
    // would also have to keep.
    struct Client {
        sf::TcpSocket socket;
        std::string name;
        color_t color = color_t::WHITE;
        std::uint32_t ply = 0;

        void sendPacket(sf::Packet &packet) {
            sf::Socket::Status status;
            do {
                status = socket.send(packet);
            } while (status == sf::Socket::Partial);
        }

        void findGame(std::uint16_t minutes) {
            FindGame request;
            request.time_limit_minutes = minutes;
            sf::Packet packet;
            packet << ClientMessage::FIND_GAME << request;
            sendPacket(packet);
        }

        void move(std::int8_t from, std::int8_t to) {
            MoveRequest request;
            request.ply = ply;
            request.move_from = from;
            request.move_to = to;
            sf::Packet packet;
            packet << ClientMessage::MOVE << request;
            sendPacket(packet);
        }

        void moveAtPly(std::uint32_t at_ply, std::int8_t from, std::int8_t to) {
            MoveRequest request;
            request.ply = at_ply;
            request.move_from = from;
            request.move_to = to;
            sf::Packet packet;
            packet << ClientMessage::MOVE << request;
            sendPacket(packet);
        }
    };

    // Steps the server until something arrives for this client, or gives up.
    std::optional<sf::Packet> Receive(GameServer &server, Client &client, int attempts = 400) {
        for (int i = 0; i < attempts; i++) {
            server.poll(1);
            sf::Packet packet;
            const sf::Socket::Status status = client.socket.receive(packet);
            if (status == sf::Socket::Done) return packet;
            if (status == sf::Socket::Disconnected) return std::nullopt;
        }
        return std::nullopt;
    }

    // Receives one message and insists it is the expected kind. The returned
    // packet is positioned just past the tag, ready for the body.
    std::optional<sf::Packet> Expect(GameServer &server, Client &client, ServerMessage expected,
                                     const std::string &what) {
        std::optional<sf::Packet> packet = Receive(server, client);
        if (!packet.has_value()) {
            Check(false, client.name + ": nothing arrived while waiting for " + what);
            return std::nullopt;
        }
        ServerMessage type;
        *packet >> type;
        if (type != expected) {
            Check(false, client.name + ": expected " + what + " but got message type " +
                         std::to_string(static_cast<int>(type)));
            return std::nullopt;
        }
        return packet;
    }

    bool Connect(Client &client, unsigned short port, const std::string &name) {
        client.name = name;
        if (client.socket.connect(sf::IpAddress::LocalHost, port, sf::seconds(2)) != sf::Socket::Done) {
            Check(false, name + " could not connect");
            return false;
        }
        client.socket.setBlocking(false);
        return true;
    }

    // Reads a GAME_START, then the GAME_STATE that follows it, and records the
    // colour the server drew for this client.
    bool StartGame(GameServer &server, Client &client) {
        std::optional<sf::Packet> start = Expect(server, client, ServerMessage::GAME_START, "game start");
        if (!start.has_value()) return false;
        GameStart message;
        *start >> message;
        client.color = message.your_color;

        std::optional<sf::Packet> state = Expect(server, client, ServerMessage::GAME_STATE, "opening state");
        if (!state.has_value()) return false;
        GameStateUpdate update;
        *state >> update;
        client.ply = update.ply;
        Check(update.fen == INITIAL_FEN, client.name + " is given the opening position");
        Check(update.ply == 0, client.name + " is given ply zero");
        Check(update.color_to_play == color_t::WHITE, client.name + " is told white moves first");
        return true;
    }

    // Plays a move as `mover` and reads the broadcast that both sides get.
    bool PlayMove(GameServer &server, Client &mover, Client &other, std::int8_t from, std::int8_t to,
                  const std::string &what) {
        mover.move(from, to);
        bool ok = true;
        for (Client *client: {&mover, &other}) {
            std::optional<sf::Packet> state = Expect(server, *client, ServerMessage::GAME_STATE, what);
            if (!state.has_value()) {
                ok = false;
                continue;
            }
            GameStateUpdate update;
            *state >> update;
            client->ply = update.ply;
            Check(update.last_move_from == from && update.last_move_to == to,
                  client->name + " is told which move was played (" + what + ")");
        }
        return ok;
    }

    std::unique_ptr<GameServer> StartServer() {
        // A fixed port can be left in TIME_WAIT by an earlier run, so a few
        // are tried before giving up.
        for (unsigned short port = 24101; port < 24111; port++) {
            auto server = std::make_unique<GameServer>(port);
            if (server->start()) return server;
        }
        return nullptr;
    }

    void TestPairingAndAGame(GameServer &server) {
        std::printf("pairing and a full game\n");
        Client first, second;
        if (!Connect(first, server.port(), "first")) return;
        server.poll(5);
        if (!Connect(second, server.port(), "second")) return;
        server.poll(5);
        Check(server.sessionCount() == 2, "the server has both sessions");

        // The first to ask has nobody to play yet.
        first.findGame(0);
        std::optional<sf::Packet> waiting = Expect(server, first, ServerMessage::WAITING, "a wait");
        Check(waiting.has_value(), "the first player is told to wait");
        Check(server.matchCount() == 0, "no match exists yet");

        second.findGame(0);
        Check(StartGame(server, first), "the first player is started");
        Check(StartGame(server, second), "the second player is started");
        Check(server.matchCount() == 1, "one match exists");
        Check(server.waitingCount() == 0, "nobody is left waiting");
        Check(first.color != second.color, "the two players are given opposite colours");

        Client &white = first.color == color_t::WHITE ? first : second;
        Client &black = first.color == color_t::WHITE ? second : first;

        // A move by the wrong player is refused, and the refusal carries the
        // real position so a client can never be left guessing.
        black.move(Sq('e', 7), Sq('e', 5));
        std::optional<sf::Packet> refused = Expect(server, black, ServerMessage::REJECTED, "a refusal");
        if (refused.has_value()) {
            Rejected rejected;
            *refused >> rejected;
            Check(rejected.reason == RejectReason::NOT_YOUR_TURN, "black is told it is not their turn");
            Check(rejected.state.fen == INITIAL_FEN, "and is given the position that really holds");
            Check(rejected.state.ply == 0, "at the right ply");
        }

        Check(PlayMove(server, white, black, Sq('f', 2), Sq('f', 3), "1. f3"), "1. f3 reaches both");
        Check(PlayMove(server, black, white, Sq('e', 7), Sq('e', 5), "1... e5"), "1... e5 reaches both");

        // An illegal move, this time from the player whose turn it really is
        // and for the position that really holds. Only the rules can refuse it.
        white.moveAtPly(2, Sq('a', 1), Sq('a', 5));
        std::optional<sf::Packet> illegal = Expect(server, white, ServerMessage::REJECTED, "a refusal");
        if (illegal.has_value()) {
            Rejected rejected;
            *illegal >> rejected;
            Check(rejected.reason == RejectReason::ILLEGAL_MOVE,
                  "a rook through its own pawn is refused as illegal");
            Check(rejected.state.ply == 2, "and the true ply comes back with it");
        }

        Check(PlayMove(server, white, black, Sq('g', 2), Sq('g', 4), "2. g4"), "2. g4 reaches both");

        // The mating move: both sides get the position, then the result.
        black.move(Sq('d', 8), Sq('h', 4));
        for (Client *client: {&black, &white}) {
            std::optional<sf::Packet> state = Expect(server, *client, ServerMessage::GAME_STATE, "the mate");
            if (state.has_value()) {
                GameStateUpdate update;
                *state >> update;
                Check(update.ply == 4, client->name + " sees ply four");
                Check(update.color_to_play == color_t::WHITE,
                      client->name + " sees white left to move");
            }
            std::optional<sf::Packet> over = Expect(server, *client, ServerMessage::GAME_OVER, "the result");
            if (over.has_value()) {
                GameOver result;
                *over >> result;
                Check(result.reason == GameOverReason::CHECKMATE, client->name + " is told it was checkmate");
                Check(result.outcome == GameOutcome::BLACK_WINS, client->name + " is told black won");
            }
        }

        Check(server.matchCount() == 0, "the finished match is cleared away");
        Check(server.sessionCount() == 2, "but both players stay connected");
    }

    void TestLeavingMidGame(GameServer &server) {
        std::printf("leaving mid game\n");
        Client stayer, leaver;
        if (!Connect(stayer, server.port(), "stayer")) return;
        server.poll(5);
        if (!Connect(leaver, server.port(), "leaver")) return;
        server.poll(5);

        stayer.findGame(0);
        Expect(server, stayer, ServerMessage::WAITING, "a wait");
        leaver.findGame(0);
        Check(StartGame(server, stayer), "the stayer is started");
        Check(StartGame(server, leaver), "the leaver is started");

        // Under peer to peer this left the other player watching a board that
        // would never change again.
        leaver.socket.disconnect();

        Expect(server, stayer, ServerMessage::GAME_STATE, "a final state");
        std::optional<sf::Packet> over = Expect(server, stayer, ServerMessage::GAME_OVER, "the result");
        if (over.has_value()) {
            GameOver result;
            *over >> result;
            Check(result.reason == GameOverReason::OPPONENT_LEFT, "the stayer is told the opponent left");
            const GameOutcome expected = stayer.color == color_t::WHITE ? GameOutcome::WHITE_WINS
                                                                        : GameOutcome::BLACK_WINS;
            Check(result.outcome == expected, "and that they won the game");
        }
        Check(server.matchCount() == 0, "the abandoned match is cleared away");
    }

    void TestResigning(GameServer &server) {
        std::printf("resigning\n");
        Client first, second;
        if (!Connect(first, server.port(), "resigner")) return;
        server.poll(5);
        if (!Connect(second, server.port(), "opponent")) return;
        server.poll(5);

        first.findGame(0);
        Expect(server, first, ServerMessage::WAITING, "a wait");
        second.findGame(0);
        Check(StartGame(server, first), "the resigner is started");
        Check(StartGame(server, second), "the opponent is started");

        sf::Packet resign;
        resign << ClientMessage::RESIGN;
        first.sendPacket(resign);

        for (Client *client: {&first, &second}) {
            Expect(server, *client, ServerMessage::GAME_STATE, "a final state");
            std::optional<sf::Packet> over = Expect(server, *client, ServerMessage::GAME_OVER, "the result");
            if (!over.has_value()) continue;
            GameOver result;
            *over >> result;
            Check(result.reason == GameOverReason::RESIGNATION, client->name + " is told it was a resignation");
            const GameOutcome expected = first.color == color_t::WHITE ? GameOutcome::BLACK_WINS
                                                                       : GameOutcome::WHITE_WINS;
            Check(result.outcome == expected, client->name + " is told the resigner lost");
        }
    }

    void TestTimeControlsArePairedSeparately(GameServer &server) {
        std::printf("separate pools per time control\n");
        Client blitz, classical;
        if (!Connect(blitz, server.port(), "blitz")) return;
        server.poll(5);
        if (!Connect(classical, server.port(), "classical")) return;
        server.poll(5);

        blitz.findGame(3);
        Expect(server, blitz, ServerMessage::WAITING, "a wait");
        classical.findGame(30);
        Expect(server, classical, ServerMessage::WAITING, "a wait");
        Check(server.matchCount() == 0, "players wanting different clocks are not paired");
        Check(server.waitingCount() == 2, "and both are left waiting");
    }

    // A client built against a different protocol version is turned away
    // rather than allowed to play a game that would behave inexplicably.
    void TestVersionMismatchIsRefused(GameServer &server) {
        std::printf("protocol version mismatch\n");
        Client stranger;
        if (!Connect(stranger, server.port(), "stranger")) return;
        server.poll(5);

        FindGame request;
        request.version = VERSION + 1;
        sf::Packet packet;
        packet << ClientMessage::FIND_GAME << request;
        stranger.sendPacket(packet);

        std::optional<sf::Packet> over = Expect(server, stranger, ServerMessage::GAME_OVER, "a refusal");
        if (over.has_value()) {
            GameOver result;
            *over >> result;
            Check(result.reason == GameOverReason::PROTOCOL_ERROR, "the reason given is a protocol error");
        }
        Check(server.matchCount() == 0, "no game is started for it");
    }

}

int main() {
    std::unique_ptr<GameServer> server = StartServer();
    if (!server) {
        std::printf("could not bind a port for the test server\n");
        return 1;
    }

    TestPairingAndAGame(*server);
    TestResigning(*server);
    TestLeavingMidGame(*server);
    TestTimeControlsArePairedSeparately(*server);
    TestVersionMismatchIsRefused(*server);

    if (failures == 0) {
        std::printf("\nall checks passed\n");
        return 0;
    }
    std::printf("\n%d check(s) failed\n", failures);
    return 1;
}
