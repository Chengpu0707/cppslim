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
    if (SlimList_GetLength(args) > 1) {
        StatementExecutor_ConstructorError(executor, "xxx");
        return NULL;
    }
    TestSlim* self = (TestSlim*)malloc(sizeof(TestSlim));
    memset(self, 0, sizeof(TestSlim));
    if (SlimList_GetLength(args) == 1)
        strncpy(self->constructionArg, SlimList_GetStringAt(args, 0), 49);
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
    return SlimList_GetStringAt(args, 0);
}

static const char* add(void*, SlimList* args)
{
    static char buf[50];
    snprintf(buf, sizeof(buf), "%s%s",
        SlimList_GetStringAt(args, 0), SlimList_GetStringAt(args, 1));
    return buf;
}

static const char* null_method(void*, SlimList*)
{
    return NULL;
}

static const char* setArg(void* self, SlimList* args)
{
    ((TestSlim*)self)->arg = SlimList_GetStringAt(args, 0);
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
    return StatementExecutor_FixtureError("my exception");
}

void TestSlim_Register(StatementExecutor* executor)
{
    StatementExecutor_RegisterFixture(executor, "TestSlim", TestSlim_Create, TestSlim_Destroy);
    StatementExecutor_RegisterMethod(executor, "TestSlim", "returnValue", returnValue);
    StatementExecutor_RegisterMethod(executor, "TestSlim", "noArgs",      noArgs);
    StatementExecutor_RegisterMethod(executor, "TestSlim", "echo",        oneArg);
    StatementExecutor_RegisterMethod(executor, "TestSlim", "add",         add);
    StatementExecutor_RegisterMethod(executor, "TestSlim", "null",        null_method);
    StatementExecutor_RegisterMethod(executor, "TestSlim", "setArg",      setArg);
    StatementExecutor_RegisterMethod(executor, "TestSlim", "getArg",      getArg);
    StatementExecutor_RegisterMethod(executor, "TestSlim", "getArg_From_Function_With_Underscores",
                                     getArg_From_Function_With_Underscores);
    StatementExecutor_RegisterMethod(executor, "TestSlim", "getConstructionArg", getConstructionArg);
    StatementExecutor_RegisterMethod(executor, "TestSlim", "returnError", returnError);

    StatementExecutor_RegisterFixture(executor, "TestSlimAgain", TestSlim_Create, TestSlim_Destroy);
    StatementExecutor_RegisterMethod(executor, "TestSlimAgain", "setArgAgain", setArg);
    StatementExecutor_RegisterMethod(executor, "TestSlimAgain", "getArgAgain", getArg);

    StatementExecutor_RegisterMethod(executor, "TestSlimDeclaredLate", "echo", oneArg);
    StatementExecutor_RegisterFixture(executor, "TestSlimDeclaredLate", TestSlim_Create, TestSlim_Destroy);

    StatementExecutor_RegisterMethod(executor, "TestSlimUndeclared", "echo", oneArg);
}
