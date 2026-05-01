#include "Slim.h"
#include "SocketServer.h"
#include "TcpComLink.h"

static Slim slim;

static int connection_handler(int socket) {
    TcpComLink comLink(socket);
    return slim.handleConnection(&comLink, &TcpComLink::send, &TcpComLink::recv);
}

int main(int /*argc*/, char** argv) {
    SocketServer server;
    server.registerHandler(&connection_handler);
    return server.run(argv[1]);
}
