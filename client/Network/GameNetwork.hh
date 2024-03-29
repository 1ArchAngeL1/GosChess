//
// Created by Leo Gogichaishvili on 31.01.22.
//

#ifndef GOSCHESS_GAMENETWORK_HH
#define GOSCHESS_GAMENETWORK_HH

#include <SFML/Network.hpp>
#include <optional>
#include <thread>
#include "../render/globals/RenderGlobals.hh"
#include "../Chess/Computation/ComputationGlobals/ComputationGlobals.hh"
#include "../Chess/Computation/ComputationGlobals/ComputationGlobals.hh"
#include "../chess/global/GameGlobals.hh"
#include "../render/menu/MenuRender.hh"
#include <any>


namespace GosChess {

    enum ConnectionType {
        HOST, CLIENT
    };

    enum TransferType {
        INITIAL,
        MOVE,
        TIMER,
        RESULT
    };

    template<typename T>
    struct DataTransfer {
        DataTransfer() = default;

        DataTransfer(TransferType protocol, T body) : protocol(protocol), body(body) {};

        TransferType protocol;
        T body;
    };

    std::optional<DataTransfer<std::any>> Receive();

    void SendMove(GosChess::action_t);

//    std::optional<GosChess::Move> ReceiveMove();

    void SendTime(GosChess::Time::TimerTransferObject);

//    std::optional<GosChess::Time::TimerTransferObject> ReceiveTime();

    void SendResult(GosChess::GameResultTransfer);

    void SetConnectionType(ConnectionType);

    void MenuNetworkMode();

    void GamePlayNetworkMode();

    void HostInit();

    void TryAccept();

    void JoinInit();

    void TryJoin();

    void InitialSend();

    void InitialReceive();

    void SetConnected(bool);

    void KillNetwork();

    extern sf::IpAddress remote_ip;

    extern sf::TcpSocket connection;

    extern sf::TcpListener client_listener;

    extern ConnectionType connection_role;

    extern bool connected;

    extern std::mutex connected_lock;

    extern bool listen_flag;

    typedef DataTransfer<GosChess::action_t> MoveTransfer;

    typedef DataTransfer<GosChess::Time::TimerTransferObject> TimeTransfer;

    typedef DataTransfer<GosChess::GameResultTransfer> ResultTransfer;

    typedef DataTransfer<GosChess::InitialTransferObject> InitialTransfer;

}


#endif //PROJECT_NAME_GAMENETWORK_H

