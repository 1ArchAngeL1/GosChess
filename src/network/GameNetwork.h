//
// Created by Leo Gogichaishvili on 31.01.22.
//

#ifndef PROJECT_NAME_GAMENETWORK_H
#define PROJECT_NAME_GAMENETWORK_H

#include <SFML/Network.hpp>
#include "../render/globals/RenderGlobals.h"
#include "../chess/computation/computation_globals/ComputationGlobals.h"
#include "../chess/computation/computation_globals/ComputationGlobals.h"
#include "../chess/global/GameGlobals.h"
#include <optional>
#include <thread>

namespace GosChess {

    enum ConnectionType {
        HOST, CLIENT
    };

    enum TransferType {
        INITIAL,
        MOVE
    };

    template<typename T>
    struct DataTransfer {
        DataTransfer() = default;

        DataTransfer(TransferType protocol, T body) : protocol(protocol), body(body) {};

        TransferType protocol;
        T body;
    };

    void SendMove(GosChess::Move);

    std::optional<GosChess::Move> ReceiveMove();

    void SetConnectionType(ConnectionType);

    void InitNewtork();

    void HostInit();

    void TryAccept();

    void JoinInit();

    void TryJoin();

    void InitialSend();

    void InitialReceive();

    void SetConnected(bool);

    extern sf::IpAddress remote_ip;

    extern sf::TcpSocket connection;

    extern sf::TcpListener client_listener;

    extern ConnectionType connection_role;

    extern bool connected;

    extern std::mutex connected_lock;

    extern bool listen_flag;

}


#endif //PROJECT_NAME_GAMENETWORK_H

