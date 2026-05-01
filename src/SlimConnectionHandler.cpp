#include "SlimConnectionHandler.h"
#include "SlimListSerializer.h"
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct SlimConnectionHandler {
    com_func_send_t sendFunc;
    com_func_recv_t recvFunc;
    void*           comLink;
    handler_func_t  slimHandlerFunc;
    void*           slimHandler;
};

SlimConnectionHandler* SlimConnectionHandler_Create(com_func_send_t sendFunction, com_func_recv_t recvFunction, void* comLink)
{
    return new SlimConnectionHandler{sendFunction, recvFunction, comLink, nullptr, nullptr};
}

void SlimConnectionHandler_Destroy(SlimConnectionHandler* self)
{
    delete self;
}

void SlimConnectionHandler_RegisterSlimMessageHandler(SlimConnectionHandler* self, void* handler, handler_func_t handlerFunc)
{
    self->slimHandler     = handler;
    self->slimHandlerFunc = handlerFunc;
}

static int read_size(SlimConnectionHandler* self)
{
    char size[7] = {};
    int receiveResult = self->recvFunc(self->comLink, size, 6);
    if (receiveResult == 6) {
        char colon;
        if (self->recvFunc(self->comLink, &colon, 1) == 1 && colon == ':')
            return atoi(size);
    } else if (receiveResult == -1) {
        return -1;
    }
    return 0;
}

int SlimConnectionHandler_Run(SlimConnectionHandler* self)
{
    if (self->sendFunc(self->comLink, "Slim -- V0.3\n", 13) == -1)
        return -1;

    while (true) {
        int size_i = read_size(self);
        if (size_i > 0) {
            std::string message(static_cast<std::size_t>(size_i), '\0');
            int numbytes = self->recvFunc(self->comLink, &message[0], size_i);
            if (numbytes != size_i) {
                printf("did not receive right number of bytes. %d expected but received %d\n", size_i, numbytes);
                break;
            }
            if (message == "bye")
                break;

            char* response_message = self->slimHandlerFunc(self->slimHandler, &message[0]);
            int   response_length  = static_cast<int>(strlen(response_message));
            char  length_buffer[8];
            sprintf(length_buffer, "%06d:", response_length);
            if (self->sendFunc(self->comLink, length_buffer, 7) == -1) {
                SlimList_Release(response_message);
                break;
            }
            self->sendFunc(self->comLink, response_message, response_length);
            SlimList_Release(response_message);
        } else if (size_i == -1) {
            break;
        }
    }
    fflush(stdout);
    return 0;
}
