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
   static void sock_init()    { WSADATA d; WSAStartup(MAKEWORD(2,2), &d); }
   static void sock_cleanup() { WSACleanup(); }
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <unistd.h>
   typedef int sock_t;
#  define SOCK_INVALID  (-1)
#  define SOCK_ERR      (-1)
#  define SOCK_CLOSE(s) close(s)
#  define SOCK_SHUT     SHUT_WR
   static void sock_init()    {}
   static void sock_cleanup() {}
#endif

SocketServer::SocketServer() = default;

SocketServer::~SocketServer()
{
    if (socket_ != -1) {
        sock_t s = static_cast<sock_t>(socket_);
        shutdown(s, SOCK_SHUT);
        SOCK_CLOSE(s);
    }
    sock_cleanup();
}

void SocketServer::registerHandler(int (*handlerFunction)(int))
{
    handler_ = handlerFunction;
}

int SocketServer::run(char* port_str)
{
    port_ = atoi(port_str);
    sock_init();

    sock_t s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == SOCK_INVALID) {
        printf("Socket creation failed.\n");
        return -1;
    }
    socket_ = static_cast<intptr_t>(s);

    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<unsigned short>(port_));
    if (bind(s, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == SOCK_ERR) {
        printf("Unable to bind socket!\n");
        return -1;
    }

    listen(s, 1);
    sock_t client = SOCK_INVALID;
    while (client == SOCK_INVALID)
        client = accept(s, nullptr, nullptr);

    (*handler_)(static_cast<int>(client));
    SOCK_CLOSE(client);

    shutdown(s, SOCK_SHUT);
    SOCK_CLOSE(s);
    socket_ = -1;
    sock_cleanup();
    return 0;
}
