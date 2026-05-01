#include "SlimConnectionHandler.h"
#include "SlimList.h"
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

SlimConnectionHandler::SlimConnectionHandler(com_func_send_t send, com_func_recv_t recv, void* comLink)
    : sendFunc_(send), recvFunc_(recv), comLink_(comLink)
{}

void SlimConnectionHandler::registerSlimMessageHandler(void* handler, handler_func_t handlerFunc)
{
    slimHandler_     = handler;
    slimHandlerFunc_ = handlerFunc;
}

int SlimConnectionHandler::readSize()
{
    char size[7] = {};
    int n = recvFunc_(comLink_, size, 6);
    if (n == 6) {
        char colon;
        if (recvFunc_(comLink_, &colon, 1) == 1 && colon == ':')
            return atoi(size);
    } else if (n == -1) {
        return -1;
    }
    return 0;
}

int SlimConnectionHandler::run()
{
    if (sendFunc_(comLink_, "Slim -- V0.3\n", 13) == -1)
        return -1;

    while (true) {
        int size = readSize();
        if (size > 0) {
            std::string message(static_cast<std::size_t>(size), '\0');
            int got = recvFunc_(comLink_, &message[0], size);
            if (got != size) {
                printf("did not receive right number of bytes. %d expected but received %d\n", size, got);
                break;
            }
            if (message == "bye")
                break;

            char* response       = slimHandlerFunc_(slimHandler_, &message[0]);
            int   responseLength = static_cast<int>(strlen(response));
            char  lenBuf[8];
            sprintf(lenBuf, "%06d:", responseLength);
            if (sendFunc_(comLink_, lenBuf, 7) == -1) {
                SlimList::release(response);
                break;
            }
            sendFunc_(comLink_, response, responseLength);
            SlimList::release(response);
        } else if (size == -1) {
            break;
        }
    }
    fflush(stdout);
    return 0;
}
