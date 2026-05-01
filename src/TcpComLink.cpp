#include "TcpComLink.h"

#ifdef _WIN32
#  include <winsock2.h>
   typedef SOCKET sock_t;
#else
#  include <sys/socket.h>
   typedef int sock_t;
#endif

TcpComLink::TcpComLink(int socket)
    : socket_(static_cast<intptr_t>(socket))
{}

int TcpComLink::send(void* voidSelf, const char* msg, int length)
{
    TcpComLink* self  = static_cast<TcpComLink*>(voidSelf);
    sock_t      sock  = static_cast<sock_t>(self->socket_);
    int total = 0;
    while (total < length) {
        int n = static_cast<int>(::send(sock, msg + total, length - total, 0));
        if (n <= 0) break;
        total += n;
    }
    return total;
}

int TcpComLink::recv(void* voidSelf, char* buffer, int length)
{
    TcpComLink* self  = static_cast<TcpComLink*>(voidSelf);
    sock_t      sock  = static_cast<sock_t>(self->socket_);
    int total = 0;
    while (total < length) {
        int n = static_cast<int>(::recv(sock, buffer + total, length - total, 0));
        if (n <= 0) break;
        total += n;
    }
    return total;
}
