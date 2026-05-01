#pragma once
#include "SlimConnectionHandler.h"
#include "StatementExecutor.h"
#include "ListExecutor.h"

class Slim {
public:
    Slim();
    ~Slim() = default;
    Slim(const Slim&) = delete;
    Slim& operator=(const Slim&) = delete;
    Slim(Slim&&) = delete;
    Slim& operator=(Slim&&) = delete;

    // Static so it can be passed as a handler_func_t callback.
    static char* handleMessage(void* voidSelf, char* message);

    int handleConnection(void* comLink, com_func_send_t send, com_func_recv_t recv);

private:
    StatementExecutor statementExecutor_;
    ListExecutor      listExecutor_;
};
