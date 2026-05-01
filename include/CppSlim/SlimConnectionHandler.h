#pragma once

typedef int   (*com_func_send_t)(void* handle, const char* msg, int length);
typedef int   (*com_func_recv_t)(void* handle, char* msg, int length);
typedef char* (*handler_func_t)(void*, char*);

class SlimConnectionHandler {
public:
    SlimConnectionHandler(com_func_send_t sendFunc, com_func_recv_t recvFunc, void* comLink);
    int  run();
    void registerSlimMessageHandler(void* handler, handler_func_t handlerFunc);

private:
    com_func_send_t sendFunc_;
    com_func_recv_t recvFunc_;
    void*           comLink_;
    handler_func_t  slimHandlerFunc_ = nullptr;
    void*           slimHandler_     = nullptr;

    int readSize();
};
