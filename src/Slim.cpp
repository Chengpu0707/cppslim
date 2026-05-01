#include "Slim.h"
#include "SlimList.h"
#include "SlimListDeserializer.h"
#include "SlimListSerializer.h"
#include "StatementExecutor.h"
#include "ListExecutor.h"

void AddFixtures(StatementExecutor*);

struct Slim {
    StatementExecutor* statementExecutor;
    ListExecutor*      listExecutor;
};

Slim* Slim_Create()
{
    Slim* self = new Slim();
    self->statementExecutor = StatementExecutor_Create();
    AddFixtures(self->statementExecutor);
    self->listExecutor = ListExecutor_Create(self->statementExecutor);
    return self;
}

void Slim_Destroy(Slim* self)
{
    ListExecutor_Destroy(self->listExecutor);
    StatementExecutor_Destroy(self->statementExecutor);
    delete self;
}

char* Slim_HandleMessage(void* voidSelf, char* message)
{
    Slim*     self         = static_cast<Slim*>(voidSelf);
    SlimList* instructions = SlimList_Deserialize(message);
    SlimList* results      = ListExecutor_Execute(self->listExecutor, instructions);
    char*     response     = SlimList_Serialize(results);
    SlimList_Destroy(results);
    SlimList_Destroy(instructions);
    return response;
}

int Slim_HandleConnection(Slim* self, void* comLink, com_func_send_t send, com_func_recv_t recv)
{
    SlimConnectionHandler* connection = SlimConnectionHandler_Create(send, recv, comLink);
    SlimConnectionHandler_RegisterSlimMessageHandler(connection, self, &Slim_HandleMessage);
    int result = SlimConnectionHandler_Run(connection);
    SlimConnectionHandler_Destroy(connection);
    return result;
}
