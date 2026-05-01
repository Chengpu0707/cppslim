#pragma once
#include <cstdint>

class SocketServer {
public:
    SocketServer();
    ~SocketServer();
    int  run(char* port);
    void registerHandler(int (*handlerFunction)(int));

private:
    int      (*handler_)(int) = nullptr;
    intptr_t socket_          = -1;   // -1 == invalid on all platforms
    int      port_            = 0;
};
