// Entry point — mirrors cslim's Main.c, reusing its protocol/transport layer.
// AddFixtures() is provided by fixtures/Fixtures.cpp.
#include "Slim.h"
#include "SocketServer.h"
#include "TcpComLink.h"

static Slim* slim;

static int connection_handler(int socket) {
    TcpComLink* comLink = TcpComLink_Create(socket);
    int result = Slim_HandleConnection(slim, comLink, &TcpComLink_send, &TcpComLink_recv);
    TcpComLink_Destroy(comLink);
    return result;
}

int main(int /*argc*/, char** argv) {
    slim = Slim_Create();
    SocketServer* server = SocketServer_Create();
    SocketServer_register_handler(server, &connection_handler);
    int result = SocketServer_Run(server, argv[1]);
    SocketServer_Destroy(server);
    Slim_Destroy(slim);
    return result;
}
