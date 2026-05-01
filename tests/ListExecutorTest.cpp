#include <gtest/gtest.h>
#include "ListExecutor.h"
#include "SlimList.h"
#include "StatementExecutor.h"
#include "TestSlim.h"

class ListExecutorTest : public ::testing::Test {
protected:
    StatementExecutor             statementExecutor;
    std::unique_ptr<ListExecutor> listExecutor;
    SlimList                      instructions;

    void SetUp() override {
        statementExecutor.addFixture(TestSlim_Register);
        listExecutor = std::make_unique<ListExecutor>(&statementExecutor);

        const char* import[] = {"i1", "import", "blah", nullptr};
        addStatementTo(&instructions, import);
        const char* make[] = {"m1", "make", "test_slim", "TestSlim", nullptr};
        addStatementTo(&instructions, make);
    }

    void addStatementTo(SlimList* list, const char** elements) {
        SlimList statement;
        while (*elements)
            statement.addString(*elements++);
        list->addList(&statement);
    }
};

TEST_F(ListExecutorTest, ImportShouldReturnOk)
{
    auto results = listExecutor->execute(&instructions);
    SlimList* importResult = results->getListAt(0);
    EXPECT_STREQ("i1", importResult->getStringAt(0));
    EXPECT_STREQ("OK", importResult->getStringAt(1));
}

TEST_F(ListExecutorTest, CannotExecuteAnInvalidOperation)
{
    const char* invalid[] = {"inv1", "Invalid", nullptr};
    addStatementTo(&instructions, invalid);
    auto results = listExecutor->execute(&instructions);
    EXPECT_EQ(3, results->getLength());
    SlimList* invalidResult = results->getListAt(2);
    EXPECT_STREQ("inv1", invalidResult->getStringAt(0));
    EXPECT_STREQ("__EXCEPTION__:message:<<INVALID_STATEMENT: [\"inv1\", \"Invalid\"].>>",
                 invalidResult->getStringAt(1));
}

TEST_F(ListExecutorTest, CanCallASimpleFunction)
{
    const char* call[] = {"call1", "call", "test_slim", "returnValue", nullptr};
    addStatementTo(&instructions, call);
    auto results = listExecutor->execute(&instructions);
    EXPECT_EQ(3, results->getLength());
    SlimList* makeResult = results->getListAt(1);
    EXPECT_STREQ("m1", makeResult->getStringAt(0));
    EXPECT_STREQ("OK", makeResult->getStringAt(1));
    SlimList* callResult = results->getListAt(2);
    EXPECT_STREQ("call1", callResult->getStringAt(0));
    EXPECT_STREQ("value", callResult->getStringAt(1));
}

TEST_F(ListExecutorTest, CantExecuteMalformedInstruction)
{
    const char* call[] = {"call1", "call", "notEnoughArguments", nullptr};
    addStatementTo(&instructions, call);
    auto results = listExecutor->execute(&instructions);
    SlimList* invalidResult = results->getListAt(2);
    EXPECT_STREQ("__EXCEPTION__:message:<<MALFORMED_INSTRUCTION [\"call1\", \"call\", \"notEnoughArguments\"].>>",
                 invalidResult->getStringAt(1));
}

TEST_F(ListExecutorTest, CantCallAMethodOnAnInstanceThatDoesntExist)
{
    const char* call[] = {"call1", "call", "noSuchInstance", "method", nullptr};
    addStatementTo(&instructions, call);
    auto results = listExecutor->execute(&instructions);
    SlimList* invalidResult = results->getListAt(2);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_INSTANCE noSuchInstance.>>",
                 invalidResult->getStringAt(1));
}

TEST_F(ListExecutorTest, ShouldRespondToAnEmptySetOfInstructionsWithAnEmptySetOfResults)
{
    SlimList emptyInstructions;
    auto results = listExecutor->execute(&emptyInstructions);
    EXPECT_EQ(0, results->getLength());
}

TEST_F(ListExecutorTest, CanPassArgumentsToConstructor)
{
    const char* make2[] = {"make2", "make", "test_slim2", "TestSlim", "ConstructorArgument", nullptr};
    const char* call[]  = {"call1", "call", "test_slim2", "getConstructionArg", nullptr};
    addStatementTo(&instructions, make2);
    addStatementTo(&instructions, call);
    auto results    = listExecutor->execute(&instructions);
    SlimList* callResult = results->getListAt(3);
    EXPECT_STREQ("ConstructorArgument", callResult->getStringAt(1));
}

TEST_F(ListExecutorTest, CanCallAFunctionMoreThanOnce)
{
    const char* call[]  = {"call1", "call", "test_slim", "echo", "Hello",   nullptr};
    const char* call2[] = {"call2", "call", "test_slim", "echo", "Goodbye", nullptr};
    addStatementTo(&instructions, call);
    addStatementTo(&instructions, call2);
    auto results = listExecutor->execute(&instructions);
    EXPECT_STREQ("Hello",   results->getListAt(2)->getStringAt(1));
    EXPECT_STREQ("Goodbye", results->getListAt(3)->getStringAt(1));
}

TEST_F(ListExecutorTest, CanAssignTheReturnValueToASymbol)
{
    const char* call[]  = {"id1", "callAndAssign", "v", "test_slim", "add", "x", "y", nullptr};
    const char* call2[] = {"id2", "call",          "test_slim", "echo", "$v",         nullptr};
    addStatementTo(&instructions, call);
    addStatementTo(&instructions, call2);
    auto results = listExecutor->execute(&instructions);
    EXPECT_STREQ("xy", results->getListAt(2)->getStringAt(1));
    EXPECT_STREQ("xy", results->getListAt(3)->getStringAt(1));
}

TEST_F(ListExecutorTest, CanReplaceMultipleSymbolsInASingleArgument)
{
    const char* c1[] = {"id1", "callAndAssign", "v1", "test_slim", "echo", "Bob",    nullptr};
    const char* c2[] = {"id2", "callAndAssign", "v2", "test_slim", "echo", "Martin", nullptr};
    const char* c3[] = {"id2", "call",          "test_slim", "echo", "name:  $v1 $v2 $12.23", nullptr};
    addStatementTo(&instructions, c1);
    addStatementTo(&instructions, c2);
    addStatementTo(&instructions, c3);
    auto results = listExecutor->execute(&instructions);
    EXPECT_STREQ("name:  Bob Martin $12.23",
                 results->getListAt(4)->getStringAt(1));
}

TEST_F(ListExecutorTest, CanPassAndReturnAList)
{
    SlimList l;
    l.addString("1");
    l.addString("2");

    SlimList statement;
    statement.addString("id1");
    statement.addString("call");
    statement.addString("test_slim");
    statement.addString("echo");
    statement.addList(&l);
    instructions.addList(&statement);

    auto results    = listExecutor->execute(&instructions);
    SlimList* callResult = results->getListAt(2);
    SlimList* resultList = callResult->getListAt(1);
    EXPECT_TRUE(l.equals(resultList));
}

TEST_F(ListExecutorTest, CanReturnNull)
{
    const char* call[] = {"id1", "call", "test_slim", "null", nullptr};
    addStatementTo(&instructions, call);
    auto results    = listExecutor->execute(&instructions);
    SlimList* callResult = results->getListAt(2);
    EXPECT_STREQ("null", callResult->getStringAt(1));
}

TEST_F(ListExecutorTest, CanPassASymbolInAList)
{
    const char* c1[] = {"id1", "callAndAssign", "v", "test_slim", "echo", "Bob", nullptr};
    addStatementTo(&instructions, c1);

    SlimList l;
    l.addString("$v");
    SlimList statement;
    statement.addString("id2");
    statement.addString("call");
    statement.addString("test_slim");
    statement.addString("echo");
    statement.addList(&l);
    instructions.addList(&statement);

    auto results    = listExecutor->execute(&instructions);
    SlimList* callResult = results->getListAt(3);
    SlimList* resultList = callResult->getListAt(1);
    SlimList expected;
    expected.addString("Bob");
    EXPECT_TRUE(expected.equals(resultList));
}
