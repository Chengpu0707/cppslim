#include "TestSlim.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct TestSlim {
    int         noArgsCalled;
    const char* arg;
    char        constructionArg[50];
    char        echoBuf[50];
};

void* TestSlim_Create(StatementExecutor* executor, SlimList* args)
{
    if (args->getLength() > 1) {
        executor->constructorError("xxx");
        return nullptr;
    }
    TestSlim* self = (TestSlim*)malloc(sizeof(TestSlim));
    memset(self, 0, sizeof(TestSlim));
    if (args->getLength() == 1)
        strncpy(self->constructionArg, args->getStringAt(0), 49);
    return self;
}

void TestSlim_Destroy(void* self)
{
    free(self);
}

int TestSlim_noArgsCalled(TestSlim* self)
{
    return self->noArgsCalled;
}

static const char* noArgs(void* self, SlimList*)
{
    ((TestSlim*)self)->noArgsCalled = 1;
    return "/__VOID__/";
}

static const char* returnValue(void*, SlimList*)
{
    return "value";
}

static const char* oneArg(void*, SlimList* args)
{
    return args->getStringAt(0);
}

static const char* add(void*, SlimList* args)
{
    static char buf[50];
    snprintf(buf, sizeof(buf), "%s%s",
        args->getStringAt(0), args->getStringAt(1));
    return buf;
}

static const char* null_method(void*, SlimList*)
{
    return nullptr;
}

static const char* setArg(void* self, SlimList* args)
{
    ((TestSlim*)self)->arg = args->getStringAt(0);
    return "/__VOID__/";
}

static const char* getArg(void* self, SlimList*)
{
    return ((TestSlim*)self)->arg;
}

static const char* getArg_From_Function_With_Underscores(void* self, SlimList*)
{
    return ((TestSlim*)self)->arg;
}

static const char* getConstructionArg(void* self, SlimList*)
{
    return ((TestSlim*)self)->constructionArg;
}

static const char* returnError(void*, SlimList*)
{
    return StatementExecutor::fixtureError("my exception");
}

void TestSlim_Register(StatementExecutor* executor)
{
    executor->registerFixture("TestSlim", TestSlim_Create, TestSlim_Destroy);
    executor->registerMethod("TestSlim", "returnValue", returnValue);
    executor->registerMethod("TestSlim", "noArgs",      noArgs);
    executor->registerMethod("TestSlim", "echo",        oneArg);
    executor->registerMethod("TestSlim", "add",         add);
    executor->registerMethod("TestSlim", "null",        null_method);
    executor->registerMethod("TestSlim", "setArg",      setArg);
    executor->registerMethod("TestSlim", "getArg",      getArg);
    executor->registerMethod("TestSlim", "getArg_From_Function_With_Underscores",
                             getArg_From_Function_With_Underscores);
    executor->registerMethod("TestSlim", "getConstructionArg", getConstructionArg);
    executor->registerMethod("TestSlim", "returnError", returnError);

    executor->registerFixture("TestSlimAgain", TestSlim_Create, TestSlim_Destroy);
    executor->registerMethod("TestSlimAgain", "setArgAgain", setArg);
    executor->registerMethod("TestSlimAgain", "getArgAgain", getArg);

    executor->registerMethod("TestSlimDeclaredLate", "echo", oneArg);
    executor->registerFixture("TestSlimDeclaredLate", TestSlim_Create, TestSlim_Destroy);

    executor->registerMethod("TestSlimUndeclared", "echo", oneArg);
}
