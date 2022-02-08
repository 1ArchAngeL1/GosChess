//
// Created by Leo Gogichaishvili on 31.01.22.
//

#include "GameNetwork.h"


sf::Packet &operator<<(sf::Packet &packet, const GosChess::Color &clr) {
    return packet << static_cast<int>(clr);
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Color &clr) {
    int temp;
    packet >> temp;
    clr = static_cast<GosChess::Color>(temp);
    return packet;
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Move &mv) {
    return packet << mv.move_from << mv.move_to;
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Move &mv) {
    return packet >> mv.move_from >> mv.move_to;
}

void GosChess::SendMove(GosChess::Move move) {
    sf::Packet packet;
    packet << move;
    while (GosChess::connection.send(packet) == sf::Socket::Partial);
}

std::optional<GosChess::Move> GosChess::ReceiveMove() {
    sf::Packet packet;
    GosChess::Move ret;
    GosChess::connection.receive(packet);
    if (packet >> ret)
        return std::optional<GosChess::Move>(ret);
    return std::nullopt;
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

void GosChess::InitHost() {
    int n = 0;
    GosChess::player_color = static_cast<GosChess::Color>(n);
    GosChess::enemy_color = static_cast<GosChess::Color>(!n);
    sf::Packet init_info;
    init_info << n;
    while (GosChess::connection.send(init_info) != sf::Socket::Done);
}

void GosChess::InitClient() {
    sf::Packet init_info;
    while (GosChess::connection.receive(init_info) != sf::Socket::Done);
    int n;
    init_info >> n;
    GosChess::player_color = static_cast<GosChess::Color>(!n);
    GosChess::enemy_color = static_cast<GosChess::Color>(n);
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
