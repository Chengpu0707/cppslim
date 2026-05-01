#pragma once
#include <cstdint>

class TcpComLink {
public:
    explicit TcpComLink(int socket);

    // Static so they can be used as com_func_send_t / com_func_recv_t callbacks.
    static int send(void* voidSelf, const char* msg, int length);
    static int recv(void* voidSelf, char* buffer, int length);

private:
    intptr_t socket_;   // holds SOCKET (Win) or int (POSIX) without platform headers
};
