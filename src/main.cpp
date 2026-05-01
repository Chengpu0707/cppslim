#include "Slim.h"
#include "SocketServer.h"
#include "TcpComLink.h"

static Slim* slim;

static int connection_handler(int socket) {
    TcpComLink* comLink = new TcpComLink(socket);
    int result = slim->handleConnection(comLink, &TcpComLink::send, &TcpComLink::recv);
    delete comLink;
    return result;
}

int main(int /*argc*/, char** argv) {
    slim = new Slim();
    SocketServer* server = new SocketServer();
    server->registerHandler(&connection_handler);
    int result = server->run(argv[1]);
    delete server;
    delete slim;
    return result;
}
