#include "TcpComLink.h"

#ifdef _WIN32
#  include <winsock2.h>
   typedef SOCKET sock_t;
#else
#  include <sys/socket.h>
   typedef int sock_t;
#endif

struct TcpComLink {
    sock_t socket;
};

TcpComLink* TcpComLink_Create(int socket)
{
    return new TcpComLink{static_cast<sock_t>(socket)};
}

void TcpComLink_Destroy(TcpComLink* self)
{
    delete self;
}

int TcpComLink_send(void* voidSelf, const char* msg, int length)
{
    TcpComLink* self = static_cast<TcpComLink*>(voidSelf);
    int total = 0;
    while (total < length) {
        int n = static_cast<int>(send(self->socket, msg + total, length - total, 0));
        if (n <= 0) break;
        total += n;
    }
    return total;
}

int TcpComLink_recv(void* voidSelf, char* buffer, int length)
{
    TcpComLink* self = static_cast<TcpComLink*>(voidSelf);
    int total = 0;
    while (total < length) {
        int n = static_cast<int>(recv(self->socket, buffer + total, length - total, 0));
        if (n <= 0) break;
        total += n;
    }
    return total;
}
