#include <gtest/gtest.h>
#include <deque>
#include <string>
#include "StatementExecutor.h"
#include "SlimList.h"
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
        args              = new SlimList();
        empty             = new SlimList();
        statementExecutor = new StatementExecutor();
        statementExecutor->addFixture(TestSlim_Register);
        statementExecutor->make("test_slim", "TestSlim", empty);
    }
    void TearDown() override {
        delete statementExecutor;
        delete args;
        delete empty;
    }
};

TEST_F(StatementExecutorTest, canCallFunctionWithNoArguments)
{
    statementExecutor->call("test_slim", "noArgs", args);
    TestSlim* ts = (TestSlim*)statementExecutor->instance("test_slim");
    EXPECT_TRUE(TestSlim_noArgsCalled(ts));
}

TEST_F(StatementExecutorTest, cantCallFunctionThatDoesNotExist)
{
    const char* result = statementExecutor->call("test_slim", "noSuchMethod", args);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_METHOD_IN_CLASS noSuchMethod[0] TestSlim.>>", result);
    result = statementExecutor->call("test_slim", "noOtherSuchMethod", args);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_METHOD_IN_CLASS noOtherSuchMethod[0] TestSlim.>>", result);
}

TEST_F(StatementExecutorTest, shouldTruncateReallyLongNamedFunction)
{
    const char* result = statementExecutor->call("test_slim",
        "noOtherSuchMethod123456789022345678903234567890423456789052345678906234567890", args);
    EXPECT_LT(strlen(result), 120u);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_METHOD_IN_CLASS noOtherSuchMethod123456789022345[0] TestSlim.>>", result);
}

TEST_F(StatementExecutorTest, shouldKnowNumberOfArgumentsForNonExistantFunction)
{
    args->addString("BlahBlah");
    const char* result = statementExecutor->call("test_slim", "noSuchMethod", args);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_METHOD_IN_CLASS noSuchMethod[1] TestSlim.>>", result);
}

TEST_F(StatementExecutorTest, shouldNotAllowACallToaNonexistentInstance)
{
    const char* result = statementExecutor->call("noSuchInstance", "noArgs", args);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_INSTANCE noSuchInstance.>>", result);
}

TEST_F(StatementExecutorTest, shouldNotAllowAMakeOnANonexistentClass)
{
    const char* result = statementExecutor->make("instanceName", "NoSuchClass", empty);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_CLASS NoSuchClass.>>", result);
}

TEST_F(StatementExecutorTest, canCallAMethodThatReturnsAValue)
{
    EXPECT_STREQ("value", statementExecutor->call("test_slim", "returnValue", args));
}

TEST_F(StatementExecutorTest, canCallAMethodThatTakesASlimList)
{
    args->addString("hello world");
    EXPECT_STREQ("hello world", statementExecutor->call("test_slim", "echo", args));
}

TEST_F(StatementExecutorTest, WhereCalledFunctionHasUnderscoresSeparatingNameParts)
{
    args->addString("hello world");
    statementExecutor->call("test_slim", "setArg", args);
    EXPECT_STREQ("hello world",
        statementExecutor->call("test_slim", "getArgFromFunctionWithUnderscores", empty));
}

TEST_F(StatementExecutorTest, canCallTwoInstancesOfTheSameFixture)
{
    SlimList* args2 = new SlimList();
    args->addString("one");
    args2->addString("two");
    statementExecutor->make("test_slim2", "TestSlim", empty);
    statementExecutor->call("test_slim",  "setArg", args);
    statementExecutor->call("test_slim2", "setArg", args2);
    const char* one = statementExecutor->call("test_slim",  "getArg", empty);
    const char* two = statementExecutor->call("test_slim2", "getArg", empty);
    EXPECT_STREQ("one", one);
    EXPECT_STREQ("two", two);
    delete args2;
}

TEST_F(StatementExecutorTest, canCreateTwoDifferentFixtures)
{
    SlimList* args2 = new SlimList();
    args->addString("one");
    args2->addString("two");
    statementExecutor->make("test_slim2", "TestSlimAgain", empty);
    statementExecutor->call("test_slim",  "setArg",     args);
    statementExecutor->call("test_slim2", "setArgAgain", args2);
    const char* one = statementExecutor->call("test_slim",  "getArg",     empty);
    const char* two = statementExecutor->call("test_slim2", "getArgAgain", empty);
    EXPECT_STREQ("one", one);
    EXPECT_STREQ("two", two);
    delete args2;
}

TEST_F(StatementExecutorTest, canReplaceSymbolsWithTheirValue)
{
    statementExecutor->setSymbol("v", "bob");
    args->addString("hi $v.");
    EXPECT_STREQ("hi bob.", statementExecutor->call("test_slim", "echo", args));
}

TEST_F(StatementExecutorTest, canReplaceSymbolsInTheMiddle)
{
    statementExecutor->setSymbol("v", "bob");
    args->addString("hi $v whats up.");
    EXPECT_STREQ("hi bob whats up.", statementExecutor->call("test_slim", "echo", args));
}

TEST_F(StatementExecutorTest, canReplaceSymbolsWithOtherNonAlphaNumeric)
{
    statementExecutor->setSymbol("v2", "bob");
    args->addString("$v2=why");
    EXPECT_STREQ("bob=why", statementExecutor->call("test_slim", "echo", args));
}

TEST_F(StatementExecutorTest, canReplaceMultipleSymbolsWithTheirValue)
{
    statementExecutor->setSymbol("v", "bob");
    statementExecutor->setSymbol("e", "doug");
    args->addString("hi $v. Cost:  $12.32 from $e.");
    EXPECT_STREQ("hi bob. Cost:  $12.32 from doug.",
        statementExecutor->call("test_slim", "echo", args));
}

TEST_F(StatementExecutorTest, canHandleStringWithJustADollarSign)
{
    args->addString("$");
    EXPECT_STREQ("$", statementExecutor->call("test_slim", "echo", args));
}

TEST_F(StatementExecutorTest, canHandleDollarSignAtTheEndOfTheString)
{
    statementExecutor->setSymbol("v2", "doug");
    args->addString("hi $v2$");
    EXPECT_STREQ("hi doug$", statementExecutor->call("test_slim", "echo", args));
}

TEST_F(StatementExecutorTest, canReplaceSymbolsInSubLists)
{
    statementExecutor->setSymbol("v2", "doug");
    SlimList* subList = new SlimList();
    subList->addString("Hi $v2.");
    args->addList(subList);
    const char* result = statementExecutor->call("test_slim", "echo", args);
    ASSERT_NE(nullptr, result);
    SlimList* returnedList = SlimList::deserialize(result);
    ASSERT_NE(nullptr, returnedList);
    EXPECT_EQ(1, returnedList->getLength());
    EXPECT_STREQ("Hi doug.", returnedList->getStringAt(0));
    delete subList;
    delete returnedList;
}

TEST_F(StatementExecutorTest, canReplaceSymbolsInSubSubLists)
{
    statementExecutor->setSymbol("v2", "doug");
    SlimList* subList    = new SlimList();
    SlimList* subSubList = new SlimList();
    subSubList->addString("Hi $v2.");
    subList->addList(subSubList);
    args->addList(subList);
    const char* result = statementExecutor->call("test_slim", "echo", args);
    ASSERT_NE(nullptr, result);
    SlimList* retSub = SlimList::deserialize(result);
    ASSERT_NE(nullptr, retSub);
    EXPECT_EQ(1, retSub->getLength());
    SlimList* retSubSub = retSub->getListAt(0);
    ASSERT_NE(nullptr, retSubSub);
    EXPECT_EQ(1, retSubSub->getLength());
    EXPECT_STREQ("Hi doug.", retSubSub->getStringAt(0));
    delete subSubList;
    delete subList;
    delete retSub;
}

TEST_F(StatementExecutorTest, canCreateFixtureWithSymbolAsClassName)
{
    statementExecutor->setSymbol("fixtureName", "Test_Slim");
    EXPECT_STREQ("OK", statementExecutor->make("instanceName", "$fixtureName", empty));
}

TEST_F(StatementExecutorTest, shouldNotAllowAMakeOnANonexistentClassReferencedBySymbol)
{
    statementExecutor->setSymbol("fixtureName", "NoSuchClass");
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_CLASS $fixtureName.>>",
        statementExecutor->make("instanceName", "$fixtureName", empty));
}

TEST_F(StatementExecutorTest, canCreateFixtureWithSymbolInClassName)
{
    statementExecutor->setSymbol("test", "Test");
    EXPECT_STREQ("OK", statementExecutor->make("instanceName", "$test_Slim", empty));
}

TEST_F(StatementExecutorTest, canCreateFixtureWithMultipleSymbolsInClassName)
{
    statementExecutor->setSymbol("test", "Test");
    statementExecutor->setSymbol("slim", "Slim");
    EXPECT_STREQ("OK", statementExecutor->make("instanceName", "$test_$slim", empty));
}

TEST_F(StatementExecutorTest, canCreateFixtureWithArguments)
{
    SlimList* constructionArgs = new SlimList();
    constructionArgs->addString("hi");
    statementExecutor->make("test_slim", "TestSlim", constructionArgs);
    EXPECT_STREQ("hi", statementExecutor->call("test_slim", "getConstructionArg", empty));
    delete constructionArgs;
}

TEST_F(StatementExecutorTest, canCreateFixtureWithArgumentsThatHaveSymbols)
{
    statementExecutor->setSymbol("name", "doug");
    SlimList* constructionArgs = new SlimList();
    constructionArgs->addString("hi $name");
    statementExecutor->make("test_slim", "TestSlim", constructionArgs);
    EXPECT_STREQ("hi doug", statementExecutor->call("test_slim", "getConstructionArg", empty));
    delete constructionArgs;
}

TEST_F(StatementExecutorTest, canCreateFixtureWithArgumentsThatHaveMultipleSymbols)
{
    statementExecutor->setSymbol("fname", "doug");
    statementExecutor->setSymbol("lname", "bradbury");
    SlimList* constructionArgs = new SlimList();
    constructionArgs->addString("hi $fname $lname");
    statementExecutor->make("test_slim", "TestSlim", constructionArgs);
    EXPECT_STREQ("hi doug bradbury", statementExecutor->call("test_slim", "getConstructionArg", empty));
    delete constructionArgs;
}

TEST_F(StatementExecutorTest, fixtureConstructionFailsWithUserErrorMessage)
{
    SlimList* constructionArgs = new SlimList();
    constructionArgs->addString("hi doug");
    constructionArgs->addString("ho doug");
    const char* result = statementExecutor->make("test_slim", "TestSlim", constructionArgs);
    EXPECT_STREQ("__EXCEPTION__:message:<<COULD_NOT_INVOKE_CONSTRUCTOR TestSlim xxx.>>", result);
    delete constructionArgs;
}

TEST_F(StatementExecutorTest, fixtureReferencedBySymbolConstructionFailsWithUserErrorMessage)
{
    statementExecutor->setSymbol("fixtureName", "Test_Slim");
    args->addString("arg0");
    args->addString("arg1");
    EXPECT_STREQ("__EXCEPTION__:message:<<COULD_NOT_INVOKE_CONSTRUCTOR $fixtureName xxx.>>",
        statementExecutor->make("instanceName", "$fixtureName", args));
}

TEST_F(StatementExecutorTest, fixtureCanReturnError)
{
    EXPECT_STREQ("__EXCEPTION__:message:<<my exception.>>",
        statementExecutor->call("test_slim", "returnError", args));
}

TEST_F(StatementExecutorTest, canCallFixtureDeclaredBackwards)
{
    statementExecutor->make("backwardsTestSlim", "TestSlimDeclaredLate", empty);
    args->addString("hi doug");
    EXPECT_STREQ("hi doug", statementExecutor->call("backwardsTestSlim", "echo", args));
}

TEST_F(StatementExecutorTest, canCallFixtureNotDeclared)
{
    statementExecutor->make("undeclaredTestSlim", "TestSlimUndeclared", empty);
    args->addString("hi doug");
    EXPECT_STREQ("hi doug", statementExecutor->call("undeclaredTestSlim", "echo", args));
}

TEST_F(StatementExecutorTest, canHaveNullResult)
{
    const char* result = statementExecutor->call("test_slim", "null", args);
    EXPECT_EQ((const char*)nullptr, result);
}

// ---------------------------------------------------------------------------
// Library instance tests
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
        statementExecutor = new StatementExecutor();
        noArgs = new SlimList();
        statementExecutor->addFixture(RegisterWith1);
        statementExecutor->addFixture(RegisterWith2);
        statementExecutor->addFixture(RegisterWith3);
    }
    void TearDown() override {
        delete noArgs;
        delete statementExecutor;
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
        ex->registerFixture("MockFixtureWith1Method", factory, dtor);
        ex->registerMethod("MockFixtureWith1Method", "method1", m1);
    }
    static void RegisterWith2(StatementExecutor* ex) {
        ex->registerFixture("MockFixtureWith2Methods", factory, dtor);
        ex->registerMethod("MockFixtureWith2Methods", "method1", m1);
        ex->registerMethod("MockFixtureWith2Methods", "method2", m2);
    }
    static void RegisterWith3(StatementExecutor* ex) {
        ex->registerFixture("MockFixtureWith3Methods", factory, dtor);
        ex->registerMethod("MockFixtureWith3Methods", "method1", m1);
        ex->registerMethod("MockFixtureWith3Methods", "method2", m2);
        ex->registerMethod("MockFixtureWith3Methods", "method3", m3);
    }
};

StatementExecutorWithLibraryInstances* StatementExecutorWithLibraryInstances::s_current = nullptr;

TEST_F(StatementExecutorWithLibraryInstances, callsMethodOnInstanceFirst)
{
    MockFixture* standard = makeInstance("OK");
    MockFixture* library  = makeInstance();
    statementExecutor->make("standardInstance", "MockFixtureWith1Method",  noArgs);
    statementExecutor->make("libraryInstance",  "MockFixtureWith2Methods", noArgs);
    EXPECT_STREQ("OK", statementExecutor->call("standardInstance", "method1", noArgs));
    EXPECT_EQ("method1", standard->lastCalledMethod);
    EXPECT_TRUE(library->lastCalledMethod.empty());
}

TEST_F(StatementExecutorWithLibraryInstances, callsMethodOnLibraryInstanceWhenNotFoundOnGivenInstance)
{
    MockFixture* standard = makeInstance();
    MockFixture* library  = makeInstance("", "OK");
    statementExecutor->make("standardInstance", "MockFixtureWith1Method",  noArgs);
    statementExecutor->make("libraryInstance",  "MockFixtureWith2Methods", noArgs);
    EXPECT_STREQ("OK", statementExecutor->call("standardInstance", "method2", noArgs));
    EXPECT_TRUE(standard->lastCalledMethod.empty());
    EXPECT_EQ("method2", library->lastCalledMethod);
}

TEST_F(StatementExecutorWithLibraryInstances, callsMethodOnTopOfLibraryInstanceStackWhenNotFoundOnGivenInstance)
{
    MockFixture* standard = makeInstance();
    MockFixture* libraryA = makeInstance();
    MockFixture* libraryB = makeInstance("", "OK");
    statementExecutor->make("standardInstance", "MockFixtureWith1Method",  noArgs);
    statementExecutor->make("libraryInstanceA", "MockFixtureWith3Methods", noArgs);
    statementExecutor->make("libraryInstanceB", "MockFixtureWith2Methods", noArgs);
    EXPECT_STREQ("OK", statementExecutor->call("standardInstance", "method2", noArgs));
    EXPECT_TRUE(standard->lastCalledMethod.empty());
    EXPECT_TRUE(libraryA->lastCalledMethod.empty());
    EXPECT_EQ("method2", libraryB->lastCalledMethod);
}

TEST_F(StatementExecutorWithLibraryInstances, callsMethodOnBottomOfLibraryInstanceStackWhenNotFoundOnGivenInstance)
{
    MockFixture* standard = makeInstance();
    MockFixture* libraryA = makeInstance("", "", "OK");
    MockFixture* libraryB = makeInstance();
    statementExecutor->make("standardInstance", "MockFixtureWith1Method",  noArgs);
    statementExecutor->make("libraryInstanceA", "MockFixtureWith3Methods", noArgs);
    statementExecutor->make("libraryInstanceB", "MockFixtureWith2Methods", noArgs);
    EXPECT_STREQ("OK", statementExecutor->call("standardInstance", "method3", noArgs));
    EXPECT_TRUE(standard->lastCalledMethod.empty());
    EXPECT_EQ("method3", libraryA->lastCalledMethod);
    EXPECT_TRUE(libraryB->lastCalledMethod.empty());
}

TEST_F(StatementExecutorWithLibraryInstances, callMethodThatDoesNotExistReturnsException)
{
    makeInstance();
    makeInstance();
    makeInstance();
    statementExecutor->make("standardInstance", "MockFixtureWith1Method",  noArgs);
    statementExecutor->make("libraryInstanceA", "MockFixtureWith3Methods", noArgs);
    statementExecutor->make("libraryInstanceB", "MockFixtureWith2Methods", noArgs);
    EXPECT_STREQ("__EXCEPTION__:message:<<NO_METHOD_IN_CLASS method4[0] MockFixtureWith1Method.>>",
        statementExecutor->call("standardInstance", "method4", noArgs));
}
