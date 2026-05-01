#include <gtest/gtest.h>
#include "SlimList.h"
#include "SlimConnectionHandler.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// ---------------------------------------------------------------------------
// Mock comlink and message handler
// ---------------------------------------------------------------------------
struct MockComLink {
    char        lastSendMsg[128];
    int         lastSendIndex;
    const char* recvStream;
    const char* recvPtr;
    SlimList*   sendReturnCodes;
};

static int mock_send_func(void* voidSelf, const char* msg, int length)
{
    MockComLink* self = (MockComLink*)voidSelf;
    strncpy(self->lastSendMsg + self->lastSendIndex, msg, length);
    self->lastSendIndex += length;

    int result = length;
    if (self->sendReturnCodes->getLength() > 0) {
        const char* s = self->sendReturnCodes->getStringAt(0);
        result = atoi(s);
        self->sendReturnCodes->popHead();
    }
    return result;
}

static int mock_recv_func(void* voidSelf, char* buffer, int length)
{
    MockComLink* self = (MockComLink*)voidSelf;
    assert(self->recvPtr != NULL);
    int result = length;
    strncpy(buffer, self->recvPtr, length);
    if ((int)strlen(self->recvPtr) < result)
        result = (int)strlen(self->recvPtr);
    self->recvPtr += result;
    if (result == 0) result = -1;
    return result;
}

static char*  g_slimResponse      = nullptr;
static char   g_sentSlimMessage[32];
static void*  g_sentMsgHandler    = nullptr;

static char* mock_handle_slim_message(void* self, char* message)
{
    strncpy(g_sentSlimMessage, message, sizeof(g_sentSlimMessage) - 1);
    g_sentMsgHandler = self;
    return g_slimResponse;
}

static void AddSendResult(MockComLink* self, int result)
{
    char string[22];
    sprintf(string, "%d", result);
    self->sendReturnCodes->addString(string);
}

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------
class SlimConnectionHandlerTest : public ::testing::Test {
protected:
    SlimConnectionHandler* slimConnectionHandler;
    MockComLink            comLink;
    void*                  mockMessageHandler;

    void SetUp() override {
        slimConnectionHandler = new SlimConnectionHandler(&mock_send_func, &mock_recv_func, (void*)&comLink);
        memset(comLink.lastSendMsg, 0, sizeof(comLink.lastSendMsg));
        comLink.lastSendIndex = 0;
        mockMessageHandler    = (void*)0x123456;
        comLink.sendReturnCodes = new SlimList();
        slimConnectionHandler->registerSlimMessageHandler(mockMessageHandler, &mock_handle_slim_message);
        g_slimResponse = nullptr;
        memset(g_sentSlimMessage, 0, sizeof(g_sentSlimMessage));
        g_sentMsgHandler = nullptr;
    }
    void TearDown() override {
        delete slimConnectionHandler;
        delete comLink.sendReturnCodes;
    }
};

TEST_F(SlimConnectionHandlerTest, ShouldSendVersion)
{
    comLink.recvStream = "000003:bye";
    comLink.recvPtr    = comLink.recvStream;

    slimConnectionHandler->run();

    EXPECT_STREQ("Slim -- V0.3\n", comLink.lastSendMsg);
}

TEST_F(SlimConnectionHandlerTest, ShouldReadMessageAndCallSlimHandler)
{
    comLink.recvStream = "000006:abcdef000003:bye";
    comLink.recvPtr    = comLink.recvStream;

    g_slimResponse = (char*)malloc(8);
    strcpy(g_slimResponse, "ghijklm");

    slimConnectionHandler->run();

    EXPECT_STREQ("Slim -- V0.3\n000007:ghijklm", comLink.lastSendMsg);
    EXPECT_STREQ("abcdef", g_sentSlimMessage);
    EXPECT_EQ(mockMessageHandler, g_sentMsgHandler);
}

TEST_F(SlimConnectionHandlerTest, CanMockSendResultsAsPartOfTest)
{
    AddSendResult(&comLink, 1);
    char message[] = "";
    int actual = mock_send_func(&comLink, message, 0);
    EXPECT_EQ(1, actual);
}

TEST_F(SlimConnectionHandlerTest, HandlesSendErrorWithoutMemoryLeak)
{
    char message[] = "";
    comLink.recvStream = message;
    comLink.recvPtr    = comLink.recvStream;
    AddSendResult(&comLink, -1);
    slimConnectionHandler->run();
}
