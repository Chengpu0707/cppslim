#include "Slim.h"
#include "SlimList.h"
#include "StatementExecutor.h"
#include "ListExecutor.h"
#include <cstring>

void AddFixtures(StatementExecutor*);

Slim::~Slim() = default;

Slim::Slim()
{
    statementExecutor_ = std::make_unique<StatementExecutor>();
    AddFixtures(statementExecutor_.get());
    listExecutor_ = std::make_unique<ListExecutor>(statementExecutor_.get());
}

char* Slim::handleMessage(void* voidSelf, char* message)
{
    Slim* self         = static_cast<Slim*>(voidSelf);
    auto  instructions = SlimList::deserialize(message);
    auto  results      = self->listExecutor_->execute(instructions.get());
    std::string response = results->serialize();
    return strdup(response.c_str());
}

int Slim::handleConnection(void* comLink, com_func_send_t send, com_func_recv_t recv)
{
    auto conn = std::make_unique<SlimConnectionHandler>(send, recv, comLink);
    conn->registerSlimMessageHandler(this, &Slim::handleMessage);
    return conn->run();
}
