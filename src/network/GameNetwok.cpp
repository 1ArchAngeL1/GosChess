//
// Created by Leo Gogichaishvili on 31.01.22.
//

#include "GameNetwok.h"


sf::Packet &operator<<(sf::Packet &packet, const GosChess::Color &clr) {
    return packet << static_cast<int>(clr);
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Color &clr) {
    return packet >> clr;
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

bool GosChess::HostGame() {
    GosChess::SetConnectionType(GosChess::ConnectionType::HOST);
    GosChess::connection.setBlocking(false);
    GosChess::client_listener.listen(2004);
    GosChess::client_listener.accept(GosChess::connection);
    return true;
}

bool GosChess::JoinGame() {
    GosChess::SetConnectionType(GosChess::ConnectionType::CLIENT);
    GosChess::connection.setBlocking(false);
    GosChess::connection.connect(GosChess::remote_ip, 2004);
    return true;
}

void GosChess::InitHost() {
    int n = 0;
    GosChess::player_color = static_cast<GosChess::Color>(n);
    GosChess::enemy_color = static_cast<GosChess::Color>(!n);
    sf::Packet init_info;
    init_info << n;
    while(GosChess::connection.send(init_info) != sf::Socket::Done);
}

void GosChess::InitClient() {
    sf::Packet init_info;
    while(GosChess::connection.receive(init_info) != sf::Socket::Done);
    int n;
    init_info >> n;
    GosChess::player_color = static_cast<GosChess::Color>(!n);
    GosChess::enemy_color = static_cast<GosChess::Color>(n);
}

sf::IpAddress GosChess::remote_ip;

sf::TcpSocket GosChess::connection;

sf::TcpListener GosChess::client_listener;

GosChess::ConnectionType GosChess::connection_role;
