#include "SocketServer.h"
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#  include <winsock2.h>
   typedef SOCKET sock_t;
#  define SOCK_INVALID  INVALID_SOCKET
#  define SOCK_ERR      SOCKET_ERROR
#  define SOCK_CLOSE(s) closesocket(s)
#  define SOCK_SHUT     SD_SEND
   static void sock_platform_init()    { WSADATA d; WSAStartup(MAKEWORD(2,2), &d); }
   static void sock_platform_cleanup() { WSACleanup(); }
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <unistd.h>
   typedef int sock_t;
#  define SOCK_INVALID  (-1)
#  define SOCK_ERR      (-1)
#  define SOCK_CLOSE(s) close(s)
#  define SOCK_SHUT     SHUT_WR
   static void sock_platform_init()    {}
   static void sock_platform_cleanup() {}
#endif

struct SocketServer {
    int (*handler)(int) = nullptr;
    sock_t socket = SOCK_INVALID;
    int port = 0;
};

SocketServer* SocketServer_Create()
{
    return new SocketServer();
}

void SocketServer_Destroy(SocketServer* self)
{
    if (self->socket != SOCK_INVALID) {
        shutdown(self->socket, SOCK_SHUT);
        SOCK_CLOSE(self->socket);
    }
    sock_platform_cleanup();
    delete self;
}

void SocketServer_register_handler(SocketServer* self, int (*handlerFunction)(int))
{
    self->handler = handlerFunction;
}

int SocketServer_Run(SocketServer* self, char* port_str)
{
    self->port = atoi(port_str);
    sock_platform_init();

    self->socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (self->socket == SOCK_INVALID) {
        printf("Socket creation failed.\n");
        return -1;
    }

    struct sockaddr_in serverInf{};
    serverInf.sin_family      = AF_INET;
    serverInf.sin_addr.s_addr = INADDR_ANY;
    serverInf.sin_port        = htons(static_cast<unsigned short>(self->port));
    if (bind(self->socket, reinterpret_cast<struct sockaddr*>(&serverInf), sizeof(serverInf)) == SOCK_ERR) {
        printf("Unable to bind socket!\n");
        return -1;
    }

    listen(self->socket, 1);
    sock_t clientSock = SOCK_INVALID;
    while (clientSock == SOCK_INVALID)
        clientSock = accept(self->socket, nullptr, nullptr);

    (*self->handler)(static_cast<int>(clientSock));
    SOCK_CLOSE(clientSock);

    shutdown(self->socket, SOCK_SHUT);
    SOCK_CLOSE(self->socket);
    self->socket = SOCK_INVALID;
    sock_platform_cleanup();
    return 0;
}
