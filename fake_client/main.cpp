//
// Created by User on 2/13/2023.
//
#include <iostream>
#include <SFML/Network.hpp>
#include <list>

int main() {
    bool send_mode = true;
    sf::TcpSocket server;
    sf::IpAddress server_ip(sf::IpAddress::LocalHost);
    if (server.connect(server_ip, 8002) == sf::Socket::Done) {

        while (true) {
            if (send_mode) {
                std::string text;
                std::cin >> text;
                sf::Packet send_packet;
                send_packet << text;
                server.send(send_packet);
                send_mode = false;
            } else {
                std::string text;
                sf::Packet input;
                while (server.receive(input) == sf::Socket::Partial);
                input >> text;
                std::cout << text << std::endl;
                send_mode = true;
            }

        }
    }

    std::cout << "can't connect to server" << std::endl;
}