#include "Slim.h"
#include "SlimList.h"
#include "StatementExecutor.h"
#include "ListExecutor.h"

void AddFixtures(StatementExecutor*);

Slim::Slim()
{
    statementExecutor_ = new StatementExecutor();
    AddFixtures(statementExecutor_);
    listExecutor_ = new ListExecutor(statementExecutor_);
}

Slim::~Slim()
{
    delete listExecutor_;
    delete statementExecutor_;
}

char* Slim::handleMessage(void* voidSelf, char* message)
{
    Slim*     self         = static_cast<Slim*>(voidSelf);
    SlimList* instructions = SlimList::deserialize(message);
    SlimList* results      = self->listExecutor_->execute(instructions);
    char*     response     = results->serialize();
    delete results;
    delete instructions;
    return response;
}

int Slim::handleConnection(void* comLink, com_func_send_t send, com_func_recv_t recv)
{
    SlimConnectionHandler* conn = new SlimConnectionHandler(send, recv, comLink);
    conn->registerSlimMessageHandler(this, &Slim::handleMessage);
    int result = conn->run();
    delete conn;
    return result;
}
