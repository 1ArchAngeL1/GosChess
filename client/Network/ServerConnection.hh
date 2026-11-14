#ifndef GOSCHESS_SERVERCONNECTION_HH
#define GOSCHESS_SERVERCONNECTION_HH

#include <cstdint>
#include <string>

#include <SFML/Network/TcpSocket.hpp>

#include "GosChess/Move.hh"
#include "GosChess/Protocol.hh"

// The client's end of the connection to the server.
//
// This replaces the old peer-to-peer layer, in which one player's copy of the
// game was the authority and the other had to take its word for everything.
// There is now one socket, to the server, and nothing here decides anything
// about the game: it sends what the player wants to do and applies what comes
// back.

namespace GosChess {

    enum class LinkState {
        OFFLINE,
        CONNECTING,
        QUEUED,       // connected and waiting for the server to find an opponent
        PLAYING,
        FAILED
    };

    // Connects and asks for a game. The connect happens on a background
    // thread so the menu carries on drawing instead of freezing on a host that
    // may not answer; watch GetLinkState() for the result.
    void JoinServer(const std::string &address, unsigned short port, std::uint16_t time_limit_minutes);

    void LeaveServer();

    LinkState GetLinkState();

    // Why the link failed, when GetLinkState() is FAILED.
    const std::string &GetLinkError();

    void SendMove(action_t move);

    void SendResign();

    // Reads everything the server has sent and applies it. Safe to call when
    // offline or still connecting, in which case it does nothing; it must be
    // called from the thread that draws.
    void ProcessServerMessages();

    // Where to connect. Held as a character buffer because ImGui's text input
    // writes straight into one.
    extern char server_address[64];

    extern int server_port;

}

#endif
