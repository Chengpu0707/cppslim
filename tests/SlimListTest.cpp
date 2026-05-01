#include <gtest/gtest.h>
#include <string.h>
#include "SlimList.h"
#include "SlimUtil.h"

class SlimListTest : public ::testing::Test {
protected:
    SlimList* slimList;
    SlimList* deserializedList;

    void SetUp() override {
        slimList         = new SlimList();
        deserializedList = nullptr;
    }
    void TearDown() override {
        delete slimList;
        delete deserializedList;
    }
    void check_lists_equal(SlimList* expected, SlimList* actual) {
        EXPECT_TRUE(expected->equals(actual));
    }
};

TEST_F(SlimListTest, twoEmptyListsAreEqual)
{
    SlimList* list = new SlimList();
    check_lists_equal(slimList, list);
    delete list;
}

TEST_F(SlimListTest, twoDifferentLengthListsAreNotEqual)
{
    SlimList* list = new SlimList();
    slimList->addString("hello");
    EXPECT_FALSE(slimList->equals(list));
    delete list;
}

TEST_F(SlimListTest, twoSingleElementListsWithDifferentElementsAreNotEqual)
{
    SlimList* list = new SlimList();
    slimList->addString("hello");
    list->addString("goodbye");
    EXPECT_FALSE(slimList->equals(list));
    delete list;
}

TEST_F(SlimListTest, twoIdenticalMultipleElementListsAreEqual)
{
    SlimList* list = new SlimList();
    slimList->addString("hello");
    slimList->addString("goodbye");
    list->addString("hello");
    list->addString("goodbye");
    EXPECT_TRUE(slimList->equals(list));
    delete list;
}

TEST_F(SlimListTest, twoNonIdenticalMultipleElementListsAreNotEqual)
{
    SlimList* list = new SlimList();
    slimList->addString("hello");
    slimList->addString("hello");
    list->addString("hello");
    list->addString("goodbye");
    EXPECT_FALSE(slimList->equals(list));
    delete list;
}

TEST_F(SlimListTest, canGetElements)
{
    slimList->addString("element1");
    slimList->addString("element2");
    EXPECT_STREQ("element1", slimList->getStringAt(0));
    EXPECT_STREQ("element2", slimList->getStringAt(1));
}

TEST_F(SlimListTest, canGetHashWithOneElement)
{
    slimList->addString("<table><tr><td>name</td><td>bob</td></tr></table>");
    SlimList* hash           = slimList->getHashAt(0);
    SlimList* twoElementList = hash->getListAt(0);
    EXPECT_STREQ("name", twoElementList->getStringAt(0));
    EXPECT_STREQ("bob",  twoElementList->getStringAt(1));
    delete hash;
}

TEST_F(SlimListTest, canGetHashWithMultipleElements)
{
    slimList->addString("<table><tr><td>name</td><td>dough</td></tr><tr><td>addr</td><td>here</td></tr></table>");
    SlimList* hash           = slimList->getHashAt(0);
    SlimList* twoElementList = hash->getListAt(1);
    EXPECT_STREQ("addr", twoElementList->getStringAt(0));
    EXPECT_STREQ("here", twoElementList->getStringAt(1));
    delete hash;
}

TEST_F(SlimListTest, cannotGetElementThatIsNotThere)
{
    slimList->addString("element1");
    slimList->addString("element2");
    EXPECT_EQ((const char*)nullptr, slimList->getStringAt(3));
}

TEST_F(SlimListTest, canReplaceString)
{
    slimList->addString("replaceMe");
    slimList->replaceAt(0, "WithMe");
    EXPECT_STREQ("WithMe", slimList->getStringAt(0));
}

TEST_F(SlimListTest, canGetTail)
{
    slimList->addString("1");
    slimList->addString("2");
    slimList->addString("3");
    slimList->addString("4");

    SlimList* expected = new SlimList();
    expected->addString("3");
    expected->addString("4");

    SlimList* tail = slimList->getTailAt(2);
    EXPECT_TRUE(expected->equals(tail));
    delete tail;
    delete expected;
}

TEST_F(SlimListTest, getDouble)
{
    slimList->addString("2.3");
    EXPECT_NEAR(2.3, slimList->getDoubleAt(0), 0.1);
}

TEST_F(SlimListTest, ToStringForEmptyList)
{
    const char* s = slimList->toString();
    EXPECT_STREQ("[]", s);
    SlimList::release(const_cast<char*>(s));
}

TEST_F(SlimListTest, toStringForSimpleList)
{
    slimList->addString("a");
    slimList->addString("b");
    const char* s = slimList->toString();
    EXPECT_STREQ("[\"a\", \"b\"]", s);
    SlimList::release(const_cast<char*>(s));
}

TEST_F(SlimListTest, toStringDoesNotHaveASideEffectWhichChangesResultsFromPriorCalls)
{
    const char* priorString = slimList->toString();
    slimList->addString("a");
    const char* withElement = slimList->toString();
    EXPECT_FALSE(strcmp(priorString, withElement) == 0);
    SlimList::release(const_cast<char*>(priorString));
    SlimList::release(const_cast<char*>(withElement));
}

TEST_F(SlimListTest, recursiveToString)
{
    slimList->addString("a");
    slimList->addString("b");
    SlimList* sublist = new SlimList();
    sublist->addString("3");
    sublist->addString("4");
    slimList->addList(sublist);
    const char* s = slimList->toString();
    EXPECT_STREQ("[\"a\", \"b\", [\"3\", \"4\"]]", s);
    SlimList::release(const_cast<char*>(s));
    delete sublist;
}

TEST_F(SlimListTest, toStringForLongList)
{
    for (int i = 0; i < 128; i++)
        slimList->addString("a");
    const char* s = slimList->toString();
    SlimList::release(const_cast<char*>(s));
}

TEST_F(SlimListTest, CanPopHeadOnListWithOneEntry)
{
    slimList->addString("a");
    slimList->popHead();
    EXPECT_EQ(0, slimList->getLength());
}

TEST_F(SlimListTest, CanInsertAfterPoppingListWithEntries)
{
    slimList->addString("a");
    slimList->addString("a");
    slimList->popHead();
    slimList->popHead();
    slimList->addString("a");
    slimList->addString("a");
    EXPECT_EQ(2, slimList->getLength());
}

TEST_F(SlimListTest, iteratorDoesNotHaveAnItemWhenEmpty)
{
    EXPECT_EQ(nullptr, slimList->createIterator());
}

TEST_F(SlimListTest, iteratorHasItem)
{
    slimList->addString("a");
    EXPECT_NE(nullptr, slimList->createIterator());
}

TEST_F(SlimListTest, iteratorNext)
{
    slimList->addString("a");
    SlimListIterator* it = slimList->createIterator();
    EXPECT_EQ(nullptr, it->advance());
}

TEST_F(SlimListTest, iteratorGetString)
{
    slimList->addString("a");
    SlimListIterator* it = slimList->createIterator();
    EXPECT_STREQ("a", it->getString());
}
