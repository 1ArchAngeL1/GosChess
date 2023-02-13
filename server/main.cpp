//
// Created by User on 2/13/2023.
//
#include <iostream>
#include <SFML/Network.hpp>
#include <list>

int main() {
    bool running = true;
    sf::TcpListener listener;
    listener.listen(8002);
    std::list<sf::TcpSocket*> clients;
    sf::SocketSelector selector;

    selector.add(listener);

    while (running)
    {
        if (selector.wait())
        {
            if (selector.isReady(listener))
            {
                sf::TcpSocket* client = new sf::TcpSocket;
                client->setBlocking(false);
                if (listener.accept(*client) == sf::Socket::Done)
                {
                    clients.push_back(client);
                    selector.add(*client);
                    std::cout << "client added" << std::endl;
                }
                else
                {
                    delete client;
                }
            }
            else
            {
                for (auto it = clients.begin(); it != clients.end(); ++it)
                {
                    sf::TcpSocket& client = **it;
                    if (selector.isReady(client))
                    {
                        sf::Packet packet;
                        if (client.receive(packet) == sf::Socket::Done)
                        {
                            std::string text;
                            packet >> text;
                            std::cout << text << std::endl;
                            sf::Packet send_packet;
                            send_packet << text;
                            for (auto ik = clients.begin(); ik != clients.end(); ++ik) {
                                if (it != ik) {
                                    (*ik)->send(packet);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}