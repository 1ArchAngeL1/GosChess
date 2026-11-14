#include <cstdlib>
#include <iostream>
#include <string>

#include "Server.hh"

int main(int argc, char **argv) {
    unsigned short port = GosChess::Protocol::DEFAULT_PORT;

    for (int i = 1; i < argc; i++) {
        const std::string argument = argv[i];
        if ((argument == "--port" || argument == "-p") && i + 1 < argc) {
            port = static_cast<unsigned short>(std::atoi(argv[++i]));
        } else {
            std::cout << "usage: " << argv[0] << " [--port n]\n"
                      << "  --port n   listen on this port (default "
                      << GosChess::Protocol::DEFAULT_PORT << ")\n";
            return argument == "--help" || argument == "-h" ? 0 : 1;
        }
    }

    GosChess::GameServer server(port);
    if (!server.start()) return 1;
    server.run();
    return 0;
}
