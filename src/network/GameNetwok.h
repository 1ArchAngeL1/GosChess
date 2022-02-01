//
// Created by Leo Gogichaishvili on 31.01.22.
//

#ifndef PROJECT_NAME_GAMENETWOK_H
#define PROJECT_NAME_GAMENETWOK_H

#include <SFML/Network.hpp>
#include "../render/globals/RenderGlobals.h"
#include "../chess_api/computation/computation_globals/ComputationGlobals.h"
#include <optional>

namespace GosChess {

    enum ConnectionType {
        HOST, CLIENT
    };

    extern void SendMove(GosChess::Move);

    extern std::optional<GosChess::Move> ReceiveMove();

    extern void SetConnectionType(ConnectionType);

    extern bool HostGame();

    extern bool JoinGame();

    extern void InitHost();

    extern void InitClient();

    extern sf::IpAddress remote_ip;

    extern sf::TcpSocket connection;

    extern sf::TcpListener client_listener;

    extern ConnectionType connection_role;

}


#endif //PROJECT_NAME_GAMENETWOK_H

