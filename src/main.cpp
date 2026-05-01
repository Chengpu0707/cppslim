#include "Slim.h"
#include "SocketServer.h"
#include "TcpComLink.h"
#include <memory>

static std::unique_ptr<Slim> slim;

static int connection_handler(int socket) {
    TcpComLink comLink(socket);
    return slim->handleConnection(&comLink, &TcpComLink::send, &TcpComLink::recv);
}

int main(int /*argc*/, char** argv) {
    slim = std::make_unique<Slim>();
    auto server = std::make_unique<SocketServer>();
    server->registerHandler(&connection_handler);
    return server->run(argv[1]);
}
