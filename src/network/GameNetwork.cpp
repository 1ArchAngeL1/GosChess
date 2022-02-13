//
// Created by Leo Gogichaishvili on 31.01.22.
//

#include "GameNetwork.h"


sf::Packet &operator<<(sf::Packet &packet, const GosChess::Color &clr) {
    return packet << static_cast<int>(clr);
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Color &clr) {
    int tmp;
    packet >> tmp;
    clr = static_cast<GosChess::Color>(tmp);
    return packet;
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::TransferType &tp) {
    return packet << static_cast<int>(tp);
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::TransferType &tp) {
    int tmp;
    packet >> tmp;
    tp = static_cast<GosChess::TransferType>(tmp);
    return packet;
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Move &mv) {
    return packet << mv.move_from << mv.move_to;
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Move &mv) {
    return packet >> mv.move_from >> mv.move_to;
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::DataTransfer<GosChess::Move> &dt) {
    return packet << dt.protocol << dt.body;
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::DataTransfer<GosChess::Move> &dt) {
    return packet >> dt.protocol >> dt.body;
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::DataTransfer<GosChess::Color> &dt) {
    return packet << dt.protocol << dt.body;
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::DataTransfer<GosChess::Color> &dt) {
    return packet >> dt.protocol >> dt.body;
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Time::TimerTransferObject &dto) {
    return packet << dto.player_timer_amount << dto.enemy_timer_amount;
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Time::TimerTransferObject &dto) {
    return packet >> dto.player_timer_amount >> dto.enemy_timer_amount;
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::DataTransfer<GosChess::Time::TimerTransferObject> &dt) {
    return packet << dt.protocol << dt.body;
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::DataTransfer<GosChess::Time::TimerTransferObject> &dt) {
    return packet >> dt.protocol >> dt.body;
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::DataTransfer<std::any> &dt) {
    packet >> dt.protocol;
    GosChess::Time::TimerTransferObject time;
    GosChess::Move move;
    switch (dt.protocol) {
        case GosChess::TransferType::TIMER:
            packet >> time;
            dt.body = static_cast<std::any>(time);
            break;
        case GosChess::TransferType::MOVE:
            packet >> move;
            dt.body = static_cast<std::any>(move);;
            break;
        default:
            break;
    }
    return packet;
}


std::optional<GosChess::DataTransfer<std::any>> GosChess::Receive() {
    sf::Packet packet;
    GosChess::DataTransfer<std::any> ret;
    GosChess::connection.receive(packet);
    if (packet >> ret)return ret;
    return std::nullopt;
}

template<typename T>
static std::optional<T> ReceiveGeneric(GosChess::TransferType type) {
    sf::Packet packet;
    GosChess::DataTransfer<T> ret;
    GosChess::connection.receive(packet);
    if (packet >> ret)
        if (ret.protocol == type) return ret.body;
    return std::nullopt;
}

void GosChess::SendMove(GosChess::Move move) {
    sf::Packet packet;
    packet << GosChess::DataTransfer<GosChess::Move>(GosChess::TransferType::MOVE, move);
    while (GosChess::connection.send(packet) == sf::Socket::Partial);
}

std::optional<GosChess::Move> GosChess::ReceiveMove() {
    return ReceiveGeneric<GosChess::Move>(GosChess::TransferType::MOVE);
}

void GosChess::SendTime(GosChess::Time::TimerTransferObject obj) {
    sf::Packet packet;
    packet << GosChess::DataTransfer<GosChess::Time::TimerTransferObject>(GosChess::TransferType::TIMER, obj);
    while (GosChess::connection.send(packet) == sf::Socket::Partial);
}

std::optional<GosChess::Time::TimerTransferObject> GosChess::ReceiveTime() {
    return ReceiveGeneric<GosChess::Time::TimerTransferObject>(GosChess::TransferType::TIMER);
}

void GosChess::SetConnectionType(GosChess::ConnectionType type) {
    GosChess::connection_role = type;
}

void GosChess::MenuNetworkMode() {
    GosChess::connection.setBlocking(true);
    GosChess::client_listener.setBlocking(false);
}

void GosChess::GamePlayNetworkMode() {
    GosChess::connection.setBlocking(false);
    GosChess::client_listener.setBlocking(false);
}

void GosChess::HostInit() {
    GosChess::SetConnectionType(GosChess::ConnectionType::HOST);
    GosChess::client_listener.listen(2001);
    GosChess::listen_flag = true;
    std::thread accept_thread(GosChess::TryAccept);
    accept_thread.detach();
}

void GosChess::TryAccept() {
    while (GosChess::client_listener.accept(GosChess::connection) != sf::Socket::Done) {
        if (!GosChess::listen_flag) {
            GosChess::KillNetwork();
            return;
        }
    }
    GosChess::SetConnected(true);
}

void GosChess::JoinInit() {
    GosChess::SetConnectionType(GosChess::ConnectionType::CLIENT);
    std::thread join_thread(GosChess::TryJoin);
    join_thread.detach();
}

void GosChess::TryJoin() {
    if (GosChess::connection.connect(GosChess::remote_ip, 2001) == sf::Socket::Done)
        GosChess::SetConnected(true);
}

void GosChess::InitialSend() {
    int n = rand() % 2;
    GosChess::player_color = static_cast<GosChess::Color>(n);
    GosChess::enemy_color = static_cast<GosChess::Color>(!n);
    sf::Packet init_info;
    init_info << GosChess::DataTransfer(GosChess::TransferType::INITIAL, enemy_color);
    while (GosChess::connection.send(init_info) == sf::Socket::Partial);
}

void GosChess::InitialReceive() {
    sf::Packet init_info;
    GosChess::DataTransfer<GosChess::Color> req;
    while (GosChess::connection.receive(init_info) == sf::Socket::Partial);
    init_info >> req;
    GosChess::player_color = static_cast<GosChess::Color>(req.body);
    GosChess::enemy_color = static_cast<GosChess::Color>(!req.body);
}

void GosChess::SetConnected(bool value) {
    std::lock_guard<std::mutex> lock(GosChess::connected_lock);
    GosChess::connected = value;
}

void GosChess::KillNetwork() {
    GosChess::connection.disconnect();
    GosChess::client_listener.close();
}

sf::IpAddress GosChess::remote_ip;

sf::TcpSocket GosChess::connection;

sf::TcpListener GosChess::client_listener;

GosChess::ConnectionType GosChess::connection_role;

bool GosChess::connected = false;

std::mutex GosChess::connected_lock;

bool GosChess::listen_flag = false;

