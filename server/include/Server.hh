#ifndef GOSCHESS_SERVER_HH
#define GOSCHESS_SERVER_HH

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <random>

#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/System/Clock.hpp>

#include "Match.hh"

namespace GosChess {

    // The authoritative server: it pairs players, owns every position and
    // every clock, and validates every move before anyone sees it.
    //
    // One thread, one socket selector, many games. Nothing blocks, so a client
    // that stops reading cannot stall anybody else's game.
    class GameServer {
    public:
        explicit GameServer(unsigned short port = Protocol::DEFAULT_PORT);

        ~GameServer();

        // Binds the listening socket. Returns false if the port is taken.
        bool start();

        // One pass of the loop: accept newcomers, read what arrived, charge
        // the clocks. Exposed separately from run() so a test can drive the
        // server by hand instead of racing a background thread.
        void poll(unsigned int wait_millis = 50);

        void run();

        void stop() { running_ = false; }

        [[nodiscard]] unsigned short port() const { return port_; }

        [[nodiscard]] std::size_t sessionCount() const { return sessions_.size(); }

        [[nodiscard]] std::size_t matchCount() const { return matches_.size(); }

        [[nodiscard]] std::size_t waitingCount() const { return waiting_by_time_limit_.size(); }

    private:
        typedef std::uint32_t session_id_t;
        typedef std::uint32_t match_id_t;

        struct Session {
            std::unique_ptr<sf::TcpSocket> socket;
            std::optional<match_id_t> match;
            // Which side this session plays in that match.
            color_t color = color_t::WHITE;
        };

        // Which session plays which side. Kept here rather than in Match so
        // that Match stays free of anything to do with connections.
        struct Players {
            session_id_t white = 0;
            session_id_t black = 0;

            [[nodiscard]] session_id_t of(color_t color) const {
                return color == color_t::WHITE ? white : black;
            }

            [[nodiscard]] session_id_t opponentOf(color_t color) const {
                return color == color_t::WHITE ? black : white;
            }
        };

        void acceptPending();

        void readFrom(session_id_t id);

        void handleFindGame(session_id_t id, sf::Packet &packet);

        void handleMove(session_id_t id, sf::Packet &packet);

        void handleResign(session_id_t id);

        void chargeClocks();

        void pair(session_id_t first, session_id_t second, std::uint16_t time_limit_minutes);

        void broadcastState(match_id_t match_id);

        void concludeMatch(match_id_t match_id);

        void send(session_id_t id, sf::Packet &packet);

        void disconnect(session_id_t id);

        void forgetWaiting(session_id_t id);

        sf::TcpListener listener_;
        sf::SocketSelector selector_;
        std::map<session_id_t, Session> sessions_;
        std::map<match_id_t, Match> matches_;
        std::map<match_id_t, Players> match_players_;
        // One player may wait per time control; the next arrival with the same
        // choice is paired with them.
        std::map<std::uint16_t, session_id_t> waiting_by_time_limit_;
        sf::Clock tick_clock_;
        // Sub-millisecond time left over from the last charge, carried to the
        // next one instead of being discarded. See chargeClocks().
        std::int64_t charge_remainder_micros_ = 0;
        std::mt19937 colour_draw_;
        unsigned short port_;
        session_id_t next_session_id_ = 1;
        match_id_t next_match_id_ = 1;
        bool running_ = false;
    };

}

#endif
