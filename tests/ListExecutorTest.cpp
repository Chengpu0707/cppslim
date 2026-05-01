#include <gtest/gtest.h>
#include "ListExecutor.h"
#include "SlimList.h"
#include "StatementExecutor.h"
#include "TestSlim.h"

class ListExecutorTest : public ::testing::Test {
protected:
    ListExecutor*      listExecutor;
    SlimList*          instructions;
    StatementExecutor* statementExecutor;

    void SetUp() override {
        statementExecutor = StatementExecutor_Create();
        StatementExecutor_AddFixture(statementExecutor, TestSlim_Register);
        listExecutor = ListExecutor_Create(statementExecutor);
        instructions = SlimList_Create();

        const char* import[] = {"i1", "import", "blah", nullptr};
        addStatementTo(instructions, import);
        const char* make[] = {"m1", "make", "test_slim", "TestSlim", nullptr};
        addStatementTo(instructions, make);
    }
    void TearDown() override {
        ListExecutor_Destroy(listExecutor);
        SlimList_Destroy(instructions);
        StatementExecutor_Destroy(statementExecutor);
    }

    void addStatementTo(SlimList* list, const char** elements) {
        SlimList* statement = SlimList_Create();
        while (*elements)
            SlimList_AddString(statement, *elements++);
        SlimList_AddList(list, statement);
        SlimList_Destroy(statement);
    }
};

TEST_F(ListExecutorTest, ImportShouldReturnOk)
{
    SlimList* results = ListExecutor_Execute(listExecutor, instructions);
    SlimList* importResult = SlimList_GetListAt(results, 0);
    EXPECT_STREQ("i1", SlimList_GetStringAt(importResult, 0));
    EXPECT_STREQ("OK", SlimList_GetStringAt(importResult, 1));
    SlimList_Destroy(results);
}

TEST_F(ListExecutorTest, CannotExecuteAnInvalidOperation)
{
    const char* invalid[] = {"inv1", "Invalid", nullptr};
    addStatementTo(instructions, invalid);
    SlimList* results = ListExecutor_Execute(listExecutor, instructions);
    EXPECT_EQ(3, SlimList_GetLength(results));
    SlimList* invalidResult = SlimList_GetListAt(results, 2);
    EXPECT_STREQ("inv1", SlimList_GetStringAt(invalidResult, 0));
    EXPECT_STREQ("__EXCEPTION__:message:<<INVALID_STATEMENT: [\"inv1\", \"Invalid\"].>>",
                 SlimList_GetStringAt(invalidResult, 1));
    SlimList_Destroy(results);
}

TEST_F(ListExecutorTest, CanCallASimpleFunction)
{
    const char* call[] = {"call1", "call", "test_slim", "returnValue", nullptr};
    addStatementTo(instructions, call);
    SlimList* results = ListExecutor_Execute(listExecutor, instructions);
    EXPECT_EQ(3, SlimList_GetLength(results));
    SlimList* makeResult = SlimList_GetListAt(results, 1);
    EXPECT_STREQ("m1", SlimList_GetStringAt(makeResult, 0));
    EXPECT_STREQ("OK", SlimList_GetStringAt(makeResult, 1));
    SlimList* callResult = SlimList_GetListAt(results, 2);
    EXPECT_STREQ("call1", SlimList_GetStringAt(callResult, 0));
    EXPECT_STREQ("value", SlimList_GetStringAt(callResult, 1));
    SlimList_Destroy(results);
}

TEST_F(ListExecutorTest, CantExecuteMalformedInstruction)
{
    const char* call[] = {"call1", "call", "notEnoughArguments", nullptr};
    addStatementTo(instructions, call);
    SlimList* results = ListExecutor_Execute(listExecutor, instructions);
    SlimList* invalidResult = SlimList_GetListAt(results, 2);
    EXPECT_STREQ("__EXCEPTION__:message:<<MALFORMED_INSTRUCTION [\"call1\", \"call\", \"notEnoughArguments\"].>>",
                 SlimList_GetStringAt(invalidResult, 1));
    SlimList_Destroy(results);
}

TEST_F(ListExecutorTest, CantCallAMethodOnAnInstanceThatDoesntExist)
{
    const char* call[] = {"call1", "call", "noSuchInstance", "method", nullptr};
    addStatementTo(instructions, call);
    SlimList* results = ListExecutor_Execute(listExecutor, instructions);
    SlimList* invalidResult = SlimList_GetListAt(results, 2);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_INSTANCE noSuchInstance.>>",
                 SlimList_GetStringAt(invalidResult, 1));
    SlimList_Destroy(results);
}

TEST_F(ListExecutorTest, ShouldRespondToAnEmptySetOfInstructionsWithAnEmptySetOfResults)
{
    SlimList* emptyInstructions = SlimList_Create();
    SlimList* results = ListExecutor_Execute(listExecutor, emptyInstructions);
    EXPECT_EQ(0, SlimList_GetLength(results));
    SlimList_Destroy(results);
    SlimList_Destroy(emptyInstructions);
}

TEST_F(ListExecutorTest, CanPassArgumentsToConstructor)
{
    const char* make2[] = {"make2", "make", "test_slim2", "TestSlim", "ConstructorArgument", nullptr};
    const char* call[]  = {"call1", "call", "test_slim2", "getConstructionArg", nullptr};
    addStatementTo(instructions, make2);
    addStatementTo(instructions, call);
    SlimList* results    = ListExecutor_Execute(listExecutor, instructions);
    SlimList* callResult = SlimList_GetListAt(results, 3);
    EXPECT_STREQ("ConstructorArgument", SlimList_GetStringAt(callResult, 1));
    SlimList_Destroy(results);
}

TEST_F(ListExecutorTest, CanCallAFunctionMoreThanOnce)
{
    const char* call[]  = {"call1", "call", "test_slim", "echo", "Hello",   nullptr};
    const char* call2[] = {"call2", "call", "test_slim", "echo", "Goodbye", nullptr};
    addStatementTo(instructions, call);
    addStatementTo(instructions, call2);
    SlimList* results = ListExecutor_Execute(listExecutor, instructions);
    EXPECT_STREQ("Hello",   SlimList_GetStringAt(SlimList_GetListAt(results, 2), 1));
    EXPECT_STREQ("Goodbye", SlimList_GetStringAt(SlimList_GetListAt(results, 3), 1));
    SlimList_Destroy(results);
}

TEST_F(ListExecutorTest, CanAssignTheReturnValueToASymbol)
{
    const char* call[]  = {"id1", "callAndAssign", "v", "test_slim", "add", "x", "y", nullptr};
    const char* call2[] = {"id2", "call",          "test_slim", "echo", "$v",         nullptr};
    addStatementTo(instructions, call);
    addStatementTo(instructions, call2);
    SlimList* results = ListExecutor_Execute(listExecutor, instructions);
    EXPECT_STREQ("xy", SlimList_GetStringAt(SlimList_GetListAt(results, 2), 1));
    EXPECT_STREQ("xy", SlimList_GetStringAt(SlimList_GetListAt(results, 3), 1));
    SlimList_Destroy(results);
}

TEST_F(ListExecutorTest, CanReplaceMultipleSymbolsInASingleArgument)
{
    const char* c1[] = {"id1", "callAndAssign", "v1", "test_slim", "echo", "Bob",    nullptr};
    const char* c2[] = {"id2", "callAndAssign", "v2", "test_slim", "echo", "Martin", nullptr};
    const char* c3[] = {"id2", "call",          "test_slim", "echo", "name:  $v1 $v2 $12.23", nullptr};
    addStatementTo(instructions, c1);
    addStatementTo(instructions, c2);
    addStatementTo(instructions, c3);
    SlimList* results = ListExecutor_Execute(listExecutor, instructions);
    EXPECT_STREQ("name:  Bob Martin $12.23",
                 SlimList_GetStringAt(SlimList_GetListAt(results, 4), 1));
    SlimList_Destroy(results);
}

TEST_F(ListExecutorTest, CanPassAndReturnAList)
{
    SlimList* l = SlimList_Create();
    SlimList_AddString(l, "1");
    SlimList_AddString(l, "2");

    SlimList* statement = SlimList_Create();
    SlimList_AddString(statement, "id1");
    SlimList_AddString(statement, "call");
    SlimList_AddString(statement, "test_slim");
    SlimList_AddString(statement, "echo");
    SlimList_AddList(statement, l);
    SlimList_AddList(instructions, statement);
    SlimList_Destroy(statement);

    SlimList* results    = ListExecutor_Execute(listExecutor, instructions);
    SlimList* callResult = SlimList_GetListAt(results, 2);
    SlimList* resultList = SlimList_GetListAt(callResult, 1);
    EXPECT_TRUE(SlimList_Equals(l, resultList));
    SlimList_Destroy(results);
    SlimList_Destroy(l);
}

TEST_F(ListExecutorTest, CanReturnNull)
{
    const char* call[] = {"id1", "call", "test_slim", "null", nullptr};
    addStatementTo(instructions, call);
    SlimList* results    = ListExecutor_Execute(listExecutor, instructions);
    SlimList* callResult = SlimList_GetListAt(results, 2);
    EXPECT_STREQ("null", SlimList_GetStringAt(callResult, 1));
    SlimList_Destroy(results);
}

TEST_F(ListExecutorTest, CanPassASymbolInAList)
{
    const char* c1[] = {"id1", "callAndAssign", "v", "test_slim", "echo", "Bob", nullptr};
    addStatementTo(instructions, c1);

    SlimList* l = SlimList_Create();
    SlimList_AddString(l, "$v");
    SlimList* statement = SlimList_Create();
    SlimList_AddString(statement, "id2");
    SlimList_AddString(statement, "call");
    SlimList_AddString(statement, "test_slim");
    SlimList_AddString(statement, "echo");
    SlimList_AddList(statement, l);
    SlimList_AddList(instructions, statement);
    SlimList_Destroy(statement);

    SlimList* results    = ListExecutor_Execute(listExecutor, instructions);
    SlimList* callResult = SlimList_GetListAt(results, 3);
    SlimList* resultList = SlimList_GetListAt(callResult, 1);
    SlimList* expected   = SlimList_Create();
    SlimList_AddString(expected, "Bob");
    EXPECT_TRUE(SlimList_Equals(expected, resultList));
    SlimList_Destroy(results);
    SlimList_Destroy(l);
    SlimList_Destroy(expected);
}
