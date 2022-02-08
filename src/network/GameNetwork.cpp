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

template<typename T>
std::optional<GosChess::Move> static ReceiveGeneric() {
    sf::Packet packet;
    GosChess::DataTransfer<T> ret;
    GosChess::connection.receive(packet);
    if (packet >> ret)
        return GosChess::DataTransfer<T>(ret).body;
    return std::nullopt;
}


void GosChess::SendMove(GosChess::Move move) {
    sf::Packet packet;
    packet << GosChess::DataTransfer<GosChess::Move>(GosChess::TransferType::MOVE,move);
    while (GosChess::connection.send(packet) == sf::Socket::Partial);
}


std::optional<GosChess::Move> GosChess::ReceiveMove() {
    return ReceiveGeneric<GosChess::Move>();
}

void GosChess::SetConnectionType(GosChess::ConnectionType type) {
    GosChess::connection_role = type;
}

void GosChess::InitNewtork() {
    GosChess::connection.setBlocking(true);
    GosChess::client_listener.setBlocking(false);
}


void GosChess::HostInit() {
    GosChess::SetConnectionType(GosChess::ConnectionType::HOST);
    GosChess::client_listener.listen(2013);
    GosChess::listen_flag = true;
    std::thread accept_thread(GosChess::TryAccept);
    accept_thread.detach();
}

void GosChess::TryAccept() {
    while (GosChess::client_listener.accept(GosChess::connection) != sf::Socket::Done) {
        if (!GosChess::listen_flag)return;
    }
    GosChess::SetConnected(true);
}

void GosChess::JoinInit() {
    GosChess::SetConnectionType(GosChess::ConnectionType::CLIENT);
    std::thread join_thread(GosChess::TryJoin);
    join_thread.detach();
}

void GosChess::TryJoin() {
    GosChess::connection.connect(GosChess::remote_ip, 2013);
    GosChess::SetConnected(true);
}

void GosChess::InitialSend() {
    int n = 0;
    GosChess::player_color = static_cast<GosChess::Color>(n);
    GosChess::enemy_color = static_cast<GosChess::Color>(!n);
    sf::Packet init_info;
    init_info << GosChess::DataTransfer(GosChess::TransferType::INITIAL,enemy_color);
    while (GosChess::connection.send(init_info) != sf::Socket::Done);
}

void GosChess::InitialReceive() {
    sf::Packet init_info;
    GosChess::DataTransfer<GosChess::Color> req;
    while (GosChess::connection.receive(init_info) != sf::Socket::Done);
    init_info >> req;
    GosChess::player_color = static_cast<GosChess::Color>(req.body);
    GosChess::enemy_color = static_cast<GosChess::Color>(!req.body);
}

void GosChess::SetConnected(bool value) {
    std::lock_guard<std::mutex> lock(GosChess::connected_lock);
    GosChess::connected = value;
}

sf::IpAddress GosChess::remote_ip;

sf::TcpSocket GosChess::connection;

sf::TcpListener GosChess::client_listener;

GosChess::ConnectionType GosChess::connection_role;

bool GosChess::connected = false;

std::mutex GosChess::connected_lock;

bool GosChess::listen_flag = false;