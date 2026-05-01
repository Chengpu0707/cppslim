#include "Slim.h"
#include "SlimList.h"
#include <cstring>

void AddFixtures(StatementExecutor*);

Slim::Slim() : listExecutor_(&statementExecutor_)
{
    AddFixtures(&statementExecutor_);
}

char* Slim::handleMessage(void* voidSelf, char* message)
{
    Slim* self         = static_cast<Slim*>(voidSelf);
    auto  instructions = SlimList::deserialize(message);
    SlimList results   = self->listExecutor_.execute(instructions.get());
    std::string response = results.serialize();
    return strdup(response.c_str());
}

int Slim::handleConnection(void* comLink, com_func_send_t send, com_func_recv_t recv)
{
    auto conn = std::make_unique<SlimConnectionHandler>(send, recv, comLink);
    conn->registerSlimMessageHandler(this, &Slim::handleMessage);
    return conn->run();
}
