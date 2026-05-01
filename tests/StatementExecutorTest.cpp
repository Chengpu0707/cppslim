#include <gtest/gtest.h>
#include <deque>
#include <string>
#include "StatementExecutor.h"
#include "SlimList.h"
#include "SlimListDeserializer.h"
#include "TestSlim.h"
#include <string.h>

// ---------------------------------------------------------------------------
// Basic StatementExecutor tests
// ---------------------------------------------------------------------------
class StatementExecutorTest : public ::testing::Test {
protected:
    StatementExecutor* statementExecutor;
    SlimList* args;
    SlimList* empty;

    void SetUp() override {
        args   = SlimList_Create();
        empty  = SlimList_Create();
        statementExecutor = StatementExecutor_Create();
        StatementExecutor_AddFixture(statementExecutor, TestSlim_Register);
        StatementExecutor_Make(statementExecutor, "test_slim", "TestSlim", empty);
    }
    void TearDown() override {
        StatementExecutor_Destroy(statementExecutor);
        SlimList_Destroy(args);
        SlimList_Destroy(empty);
    }
};

TEST_F(StatementExecutorTest, canCallFunctionWithNoArguments)
{
    StatementExecutor_Call(statementExecutor, "test_slim", "noArgs", args);
    TestSlim* ts = (TestSlim*)StatementExecutor_Instance(statementExecutor, "test_slim");
    EXPECT_TRUE(TestSlim_noArgsCalled(ts));
}

TEST_F(StatementExecutorTest, cantCallFunctionThatDoesNotExist)
{
    const char* result = StatementExecutor_Call(statementExecutor, "test_slim", "noSuchMethod", args);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_METHOD_IN_CLASS noSuchMethod[0] TestSlim.>>", result);
    result = StatementExecutor_Call(statementExecutor, "test_slim", "noOtherSuchMethod", args);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_METHOD_IN_CLASS noOtherSuchMethod[0] TestSlim.>>", result);
}

TEST_F(StatementExecutorTest, shouldTruncateReallyLongNamedFunction)
{
    const char* result = StatementExecutor_Call(statementExecutor, "test_slim",
        "noOtherSuchMethod123456789022345678903234567890423456789052345678906234567890", args);
    EXPECT_LT(strlen(result), 120u);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_METHOD_IN_CLASS noOtherSuchMethod123456789022345[0] TestSlim.>>", result);
}

TEST_F(StatementExecutorTest, shouldKnowNumberOfArgumentsForNonExistantFunction)
{
    SlimList_AddString(args, "BlahBlah");
    const char* result = StatementExecutor_Call(statementExecutor, "test_slim", "noSuchMethod", args);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_METHOD_IN_CLASS noSuchMethod[1] TestSlim.>>", result);
}

TEST_F(StatementExecutorTest, shouldNotAllowACallToaNonexistentInstance)
{
    const char* result = StatementExecutor_Call(statementExecutor, "noSuchInstance", "noArgs", args);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_INSTANCE noSuchInstance.>>", result);
}

TEST_F(StatementExecutorTest, shouldNotAllowAMakeOnANonexistentClass)
{
    const char* result = StatementExecutor_Make(statementExecutor, "instanceName", "NoSuchClass", empty);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_CLASS NoSuchClass.>>", result);
}

TEST_F(StatementExecutorTest, canCallAMethodThatReturnsAValue)
{
    const char* result = StatementExecutor_Call(statementExecutor, "test_slim", "returnValue", args);
    EXPECT_STREQ("value", result);
}

TEST_F(StatementExecutorTest, canCallAMethodThatTakesASlimList)
{
    SlimList_AddString(args, "hello world");
    const char* result = StatementExecutor_Call(statementExecutor, "test_slim", "echo", args);
    EXPECT_STREQ("hello world", result);
}

TEST_F(StatementExecutorTest, WhereCalledFunctionHasUnderscoresSeparatingNameParts)
{
    SlimList_AddString(args, "hello world");
    StatementExecutor_Call(statementExecutor, "test_slim", "setArg", args);
    const char* result = StatementExecutor_Call(statementExecutor, "test_slim", "getArgFromFunctionWithUnderscores", empty);
    EXPECT_STREQ("hello world", result);
}

TEST_F(StatementExecutorTest, canCallTwoInstancesOfTheSameFixture)
{
    SlimList* args2 = SlimList_Create();
    SlimList_AddString(args,  "one");
    SlimList_AddString(args2, "two");
    StatementExecutor_Make(statementExecutor, "test_slim2", "TestSlim", empty);
    StatementExecutor_Call(statementExecutor, "test_slim",  "setArg", args);
    StatementExecutor_Call(statementExecutor, "test_slim2", "setArg", args2);
    const char* one = StatementExecutor_Call(statementExecutor, "test_slim",  "getArg", empty);
    const char* two = StatementExecutor_Call(statementExecutor, "test_slim2", "getArg", empty);
    EXPECT_STREQ("one", one);
    EXPECT_STREQ("two", two);
    SlimList_Destroy(args2);
}

TEST_F(StatementExecutorTest, canCreateTwoDifferentFixtures)
{
    SlimList* args2 = SlimList_Create();
    SlimList_AddString(args,  "one");
    SlimList_AddString(args2, "two");
    StatementExecutor_Make(statementExecutor, "test_slim2", "TestSlimAgain", empty);
    StatementExecutor_Call(statementExecutor, "test_slim",  "setArg",     args);
    StatementExecutor_Call(statementExecutor, "test_slim2", "setArgAgain", args2);
    const char* one = StatementExecutor_Call(statementExecutor, "test_slim",  "getArg",     empty);
    const char* two = StatementExecutor_Call(statementExecutor, "test_slim2", "getArgAgain", empty);
    EXPECT_STREQ("one", one);
    EXPECT_STREQ("two", two);
    SlimList_Destroy(args2);
}

TEST_F(StatementExecutorTest, canReplaceSymbolsWithTheirValue)
{
    StatementExecutor_SetSymbol(statementExecutor, "v", "bob");
    SlimList_AddString(args, "hi $v.");
    const char* result = StatementExecutor_Call(statementExecutor, "test_slim", "echo", args);
    EXPECT_EQ(strlen("hi bob."), strlen(result));
    EXPECT_STREQ("hi bob.", result);
}

TEST_F(StatementExecutorTest, canReplaceSymbolsInTheMiddle)
{
    StatementExecutor_SetSymbol(statementExecutor, "v", "bob");
    SlimList_AddString(args, "hi $v whats up.");
    EXPECT_STREQ("hi bob whats up.", StatementExecutor_Call(statementExecutor, "test_slim", "echo", args));
}

TEST_F(StatementExecutorTest, canReplaceSymbolsWithOtherNonAlphaNumeric)
{
    StatementExecutor_SetSymbol(statementExecutor, "v2", "bob");
    SlimList_AddString(args, "$v2=why");
    EXPECT_STREQ("bob=why", StatementExecutor_Call(statementExecutor, "test_slim", "echo", args));
}

TEST_F(StatementExecutorTest, canReplaceMultipleSymbolsWithTheirValue)
{
    StatementExecutor_SetSymbol(statementExecutor, "v", "bob");
    StatementExecutor_SetSymbol(statementExecutor, "e", "doug");
    SlimList_AddString(args, "hi $v. Cost:  $12.32 from $e.");
    EXPECT_STREQ("hi bob. Cost:  $12.32 from doug.",
        StatementExecutor_Call(statementExecutor, "test_slim", "echo", args));
}

TEST_F(StatementExecutorTest, canHandleStringWithJustADollarSign)
{
    StatementExecutor_SetSymbol(statementExecutor, "v2", "bob");
    SlimList_AddString(args, "$");
    EXPECT_STREQ("$", StatementExecutor_Call(statementExecutor, "test_slim", "echo", args));
}

TEST_F(StatementExecutorTest, canHandleDollarSignAtTheEndOfTheString)
{
    StatementExecutor_SetSymbol(statementExecutor, "v2", "doug");
    SlimList_AddString(args, "hi $v2$");
    EXPECT_STREQ("hi doug$", StatementExecutor_Call(statementExecutor, "test_slim", "echo", args));
}

TEST_F(StatementExecutorTest, canReplaceSymbolsInSubLists)
{
    StatementExecutor_SetSymbol(statementExecutor, "v2", "doug");
    SlimList* subList = SlimList_Create();
    SlimList_AddString(subList, "Hi $v2.");
    SlimList_AddList(args, subList);
    const char* result = StatementExecutor_Call(statementExecutor, "test_slim", "echo", args);
    ASSERT_NE(nullptr, result);
    SlimList* returnedList = SlimList_Deserialize(result);
    ASSERT_NE(nullptr, returnedList);
    EXPECT_EQ(1, SlimList_GetLength(returnedList));
    EXPECT_STREQ("Hi doug.", SlimList_GetStringAt(returnedList, 0));
    SlimList_Destroy(subList);
    SlimList_Destroy(returnedList);
}

TEST_F(StatementExecutorTest, canReplaceSymbolsInSubSubLists)
{
    StatementExecutor_SetSymbol(statementExecutor, "v2", "doug");
    SlimList* subList    = SlimList_Create();
    SlimList* subSubList = SlimList_Create();
    SlimList_AddString(subSubList, "Hi $v2.");
    SlimList_AddList(subList, subSubList);
    SlimList_AddList(args, subList);
    const char* result = StatementExecutor_Call(statementExecutor, "test_slim", "echo", args);
    ASSERT_NE(nullptr, result);
    SlimList* retSub = SlimList_Deserialize(result);
    ASSERT_NE(nullptr, retSub);
    EXPECT_EQ(1, SlimList_GetLength(retSub));
    SlimList* retSubSub = SlimList_GetListAt(retSub, 0);
    ASSERT_NE(nullptr, retSubSub);
    EXPECT_EQ(1, SlimList_GetLength(retSubSub));
    EXPECT_STREQ("Hi doug.", SlimList_GetStringAt(retSubSub, 0));
    SlimList_Destroy(subSubList);
    SlimList_Destroy(subList);
    SlimList_Destroy(retSub);
}

TEST_F(StatementExecutorTest, canCreateFixtureWithSymbolAsClassName)
{
    StatementExecutor_SetSymbol(statementExecutor, "fixtureName", "Test_Slim");
    EXPECT_STREQ("OK", StatementExecutor_Make(statementExecutor, "instanceName", "$fixtureName", empty));
}

TEST_F(StatementExecutorTest, shouldNotAllowAMakeOnANonexistentClassReferencedBySymbol)
{
    StatementExecutor_SetSymbol(statementExecutor, "fixtureName", "NoSuchClass");
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_CLASS $fixtureName.>>",
        StatementExecutor_Make(statementExecutor, "instanceName", "$fixtureName", empty));
}

TEST_F(StatementExecutorTest, canCreateFixtureWithSymbolInClassName)
{
    StatementExecutor_SetSymbol(statementExecutor, "test", "Test");
    EXPECT_STREQ("OK", StatementExecutor_Make(statementExecutor, "instanceName", "$test_Slim", empty));
}

TEST_F(StatementExecutorTest, canCreateFixtureWithMultipleSymbolsInClassName)
{
    StatementExecutor_SetSymbol(statementExecutor, "test", "Test");
    StatementExecutor_SetSymbol(statementExecutor, "slim", "Slim");
    EXPECT_STREQ("OK", StatementExecutor_Make(statementExecutor, "instanceName", "$test_$slim", empty));
}

TEST_F(StatementExecutorTest, canCreateFixtureWithArguments)
{
    SlimList* constructionArgs = SlimList_Create();
    SlimList_AddString(constructionArgs, "hi");
    StatementExecutor_Make(statementExecutor, "test_slim", "TestSlim", constructionArgs);
    EXPECT_STREQ("hi", StatementExecutor_Call(statementExecutor, "test_slim", "getConstructionArg", empty));
    SlimList_Destroy(constructionArgs);
}

TEST_F(StatementExecutorTest, canCreateFixtureWithArgumentsThatHaveSymbols)
{
    StatementExecutor_SetSymbol(statementExecutor, "name", "doug");
    SlimList* constructionArgs = SlimList_Create();
    SlimList_AddString(constructionArgs, "hi $name");
    StatementExecutor_Make(statementExecutor, "test_slim", "TestSlim", constructionArgs);
    EXPECT_STREQ("hi doug", StatementExecutor_Call(statementExecutor, "test_slim", "getConstructionArg", empty));
    SlimList_Destroy(constructionArgs);
}

TEST_F(StatementExecutorTest, canCreateFixtureWithArgumentsThatHaveMultipleSymbols)
{
    StatementExecutor_SetSymbol(statementExecutor, "fname", "doug");
    StatementExecutor_SetSymbol(statementExecutor, "lname", "bradbury");
    SlimList* constructionArgs = SlimList_Create();
    SlimList_AddString(constructionArgs, "hi $fname $lname");
    StatementExecutor_Make(statementExecutor, "test_slim", "TestSlim", constructionArgs);
    EXPECT_STREQ("hi doug bradbury", StatementExecutor_Call(statementExecutor, "test_slim", "getConstructionArg", empty));
    SlimList_Destroy(constructionArgs);
}

TEST_F(StatementExecutorTest, fixtureConstructionFailsWithUserErrorMessage)
{
    SlimList* constructionArgs = SlimList_Create();
    SlimList_AddString(constructionArgs, "hi doug");
    SlimList_AddString(constructionArgs, "ho doug");
    const char* result = StatementExecutor_Make(statementExecutor, "test_slim", "TestSlim", constructionArgs);
    EXPECT_STREQ("__EXCEPTION__:message:<<COULD_NOT_INVOKE_CONSTRUCTOR TestSlim xxx.>>", result);
    SlimList_Destroy(constructionArgs);
}

TEST_F(StatementExecutorTest, fixtureReferencedBySymbolConstructionFailsWithUserErrorMessage)
{
    StatementExecutor_SetSymbol(statementExecutor, "fixtureName", "Test_Slim");
    SlimList_AddString(args, "arg0");
    SlimList_AddString(args, "arg1");
    EXPECT_STREQ("__EXCEPTION__:message:<<COULD_NOT_INVOKE_CONSTRUCTOR $fixtureName xxx.>>",
        StatementExecutor_Make(statementExecutor, "instanceName", "$fixtureName", args));
}

TEST_F(StatementExecutorTest, fixtureCanReturnError)
{
    const char* result = StatementExecutor_Call(statementExecutor, "test_slim", "returnError", args);
    EXPECT_STREQ("__EXCEPTION__:message:<<my exception.>>", result);
}

TEST_F(StatementExecutorTest, canCallFixtureDeclaredBackwards)
{
    StatementExecutor_Make(statementExecutor, "backwardsTestSlim", "TestSlimDeclaredLate", empty);
    SlimList_AddString(args, "hi doug");
    EXPECT_STREQ("hi doug", StatementExecutor_Call(statementExecutor, "backwardsTestSlim", "echo", args));
}

TEST_F(StatementExecutorTest, canCallFixtureNotDeclared)
{
    StatementExecutor_Make(statementExecutor, "undeclaredTestSlim", "TestSlimUndeclared", empty);
    SlimList_AddString(args, "hi doug");
    EXPECT_STREQ("hi doug", StatementExecutor_Call(statementExecutor, "undeclaredTestSlim", "echo", args));
}

TEST_F(StatementExecutorTest, canHaveNullResult)
{
    const char* result = StatementExecutor_Call(statementExecutor, "test_slim", "null", args);
    EXPECT_EQ((const char*)nullptr, result);
}

// ---------------------------------------------------------------------------
// Library instance tests (replaces CppUTestExt mock with a simple tracker)
// ---------------------------------------------------------------------------
struct MockFixture {
    std::string lastCalledMethod;
    std::string m1_ret, m2_ret, m3_ret;
    MockFixture(const std::string& r1 = "", const std::string& r2 = "", const std::string& r3 = "")
        : m1_ret(r1), m2_ret(r2), m3_ret(r3) {}
};

class StatementExecutorWithLibraryInstances : public ::testing::Test {
    static StatementExecutorWithLibraryInstances* s_current;
protected:
    StatementExecutor* statementExecutor;
    SlimList* noArgs;
    std::deque<MockFixture*> createQueue;

    void SetUp() override {
        s_current = this;
        statementExecutor = StatementExecutor_Create();
        noArgs = SlimList_Create();
        StatementExecutor_AddFixture(statementExecutor, RegisterWith1);
        StatementExecutor_AddFixture(statementExecutor, RegisterWith2);
        StatementExecutor_AddFixture(statementExecutor, RegisterWith3);
    }
    void TearDown() override {
        SlimList_Destroy(noArgs);
        StatementExecutor_Destroy(statementExecutor);
        s_current = nullptr;
    }

    MockFixture* makeInstance(const std::string& r1 = "", const std::string& r2 = "", const std::string& r3 = "") {
        auto* f = new MockFixture(r1, r2, r3);
        createQueue.push_back(f);
        return f;
    }

    static void* factory(StatementExecutor*, SlimList*) {
        EXPECT_FALSE(s_current->createQueue.empty());
        MockFixture* f = s_current->createQueue.front();
        s_current->createQueue.pop_front();
        return f;
    }
    static void dtor(void* p) { delete static_cast<MockFixture*>(p); }

    static const char* m1(void* obj, SlimList*) {
        auto* f = static_cast<MockFixture*>(obj);
        f->lastCalledMethod = "method1";
        return f->m1_ret.c_str();
    }
    static const char* m2(void* obj, SlimList*) {
        auto* f = static_cast<MockFixture*>(obj);
        f->lastCalledMethod = "method2";
        return f->m2_ret.c_str();
    }
    static const char* m3(void* obj, SlimList*) {
        auto* f = static_cast<MockFixture*>(obj);
        f->lastCalledMethod = "method3";
        return f->m3_ret.c_str();
    }

    static void RegisterWith1(StatementExecutor* ex) {
        StatementExecutor_RegisterFixture(ex, "MockFixtureWith1Method", factory, dtor);
        StatementExecutor_RegisterMethod(ex, "MockFixtureWith1Method", "method1", m1);
    }
    static void RegisterWith2(StatementExecutor* ex) {
        StatementExecutor_RegisterFixture(ex, "MockFixtureWith2Methods", factory, dtor);
        StatementExecutor_RegisterMethod(ex, "MockFixtureWith2Methods", "method1", m1);
        StatementExecutor_RegisterMethod(ex, "MockFixtureWith2Methods", "method2", m2);
    }
    static void RegisterWith3(StatementExecutor* ex) {
        StatementExecutor_RegisterFixture(ex, "MockFixtureWith3Methods", factory, dtor);
        StatementExecutor_RegisterMethod(ex, "MockFixtureWith3Methods", "method1", m1);
        StatementExecutor_RegisterMethod(ex, "MockFixtureWith3Methods", "method2", m2);
        StatementExecutor_RegisterMethod(ex, "MockFixtureWith3Methods", "method3", m3);
    }
};

StatementExecutorWithLibraryInstances* StatementExecutorWithLibraryInstances::s_current = nullptr;

TEST_F(StatementExecutorWithLibraryInstances, callsMethodOnInstanceFirst)
{
    MockFixture* standard = makeInstance("OK");
    MockFixture* library  = makeInstance();

    StatementExecutor_Make(statementExecutor, "standardInstance", "MockFixtureWith1Method",  noArgs);
    StatementExecutor_Make(statementExecutor, "libraryInstance",  "MockFixtureWith2Methods", noArgs);
    const char* result = StatementExecutor_Call(statementExecutor, "standardInstance", "method1", noArgs);

    EXPECT_STREQ("OK", result);
    EXPECT_EQ("method1", standard->lastCalledMethod);
    EXPECT_TRUE(library->lastCalledMethod.empty());
}

TEST_F(StatementExecutorWithLibraryInstances, callsMethodOnLibraryInstanceWhenNotFoundOnGivenInstance)
{
    MockFixture* standard = makeInstance();
    MockFixture* library  = makeInstance("", "OK");

    StatementExecutor_Make(statementExecutor, "standardInstance", "MockFixtureWith1Method",  noArgs);
    StatementExecutor_Make(statementExecutor, "libraryInstance",  "MockFixtureWith2Methods", noArgs);
    const char* result = StatementExecutor_Call(statementExecutor, "standardInstance", "method2", noArgs);

    EXPECT_STREQ("OK", result);
    EXPECT_TRUE(standard->lastCalledMethod.empty());
    EXPECT_EQ("method2", library->lastCalledMethod);
}

TEST_F(StatementExecutorWithLibraryInstances, callsMethodOnTopOfLibraryInstanceStackWhenNotFoundOnGivenInstance)
{
    MockFixture* standard  = makeInstance();
    MockFixture* libraryA  = makeInstance();
    MockFixture* libraryB  = makeInstance("", "OK");

    StatementExecutor_Make(statementExecutor, "standardInstance",  "MockFixtureWith1Method",  noArgs);
    StatementExecutor_Make(statementExecutor, "libraryInstanceA",  "MockFixtureWith3Methods", noArgs);
    StatementExecutor_Make(statementExecutor, "libraryInstanceB",  "MockFixtureWith2Methods", noArgs);
    const char* result = StatementExecutor_Call(statementExecutor, "standardInstance", "method2", noArgs);

    EXPECT_STREQ("OK", result);
    EXPECT_TRUE(standard->lastCalledMethod.empty());
    EXPECT_TRUE(libraryA->lastCalledMethod.empty());
    EXPECT_EQ("method2", libraryB->lastCalledMethod);
}

TEST_F(StatementExecutorWithLibraryInstances, callsMethodOnBottomOfLibraryInstanceStackWhenNotFoundOnGivenInstance)
{
    MockFixture* standard  = makeInstance();
    MockFixture* libraryA  = makeInstance("", "", "OK");
    MockFixture* libraryB  = makeInstance();

    StatementExecutor_Make(statementExecutor, "standardInstance",  "MockFixtureWith1Method",  noArgs);
    StatementExecutor_Make(statementExecutor, "libraryInstanceA",  "MockFixtureWith3Methods", noArgs);
    StatementExecutor_Make(statementExecutor, "libraryInstanceB",  "MockFixtureWith2Methods", noArgs);
    const char* result = StatementExecutor_Call(statementExecutor, "standardInstance", "method3", noArgs);

    EXPECT_STREQ("OK", result);
    EXPECT_TRUE(standard->lastCalledMethod.empty());
    EXPECT_EQ("method3", libraryA->lastCalledMethod);
    EXPECT_TRUE(libraryB->lastCalledMethod.empty());
}

TEST_F(StatementExecutorWithLibraryInstances, callMethodThatDoesNotExistReturnsException)
{
    MockFixture* standard  = makeInstance();
    MockFixture* libraryA  = makeInstance();
    MockFixture* libraryB  = makeInstance();

    StatementExecutor_Make(statementExecutor, "standardInstance",  "MockFixtureWith1Method",  noArgs);
    StatementExecutor_Make(statementExecutor, "libraryInstanceA",  "MockFixtureWith3Methods", noArgs);
    StatementExecutor_Make(statementExecutor, "libraryInstanceB",  "MockFixtureWith2Methods", noArgs);
    const char* result = StatementExecutor_Call(statementExecutor, "standardInstance", "method4", noArgs);

    EXPECT_STREQ("__EXCEPTION__:message:<<NO_METHOD_IN_CLASS method4[0] MockFixtureWith1Method.>>", result);
    EXPECT_TRUE(standard->lastCalledMethod.empty());
    EXPECT_TRUE(libraryA->lastCalledMethod.empty());
    EXPECT_TRUE(libraryB->lastCalledMethod.empty());
}
