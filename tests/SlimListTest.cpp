#include <gtest/gtest.h>
#include <string.h>
#include "SlimList.h"
#include "SlimUtil.h"

class SlimListTest : public ::testing::Test {
protected:
    SlimList slimList;
    void check_lists_equal(const SlimList* expected, const SlimList* actual) {
        EXPECT_TRUE(expected->equals(actual));
    }
};

TEST_F(SlimListTest, twoEmptyListsAreEqual)
{
    SlimList list;
    check_lists_equal(&slimList, &list);
}

TEST_F(SlimListTest, twoDifferentLengthListsAreNotEqual)
{
    SlimList list;
    slimList.addString("hello");
    EXPECT_FALSE(slimList.equals(&list));
}

TEST_F(SlimListTest, twoSingleElementListsWithDifferentElementsAreNotEqual)
{
    SlimList list;
    slimList.addString("hello");
    list.addString("goodbye");
    EXPECT_FALSE(slimList.equals(&list));
}

TEST_F(SlimListTest, twoIdenticalMultipleElementListsAreEqual)
{
    SlimList list;
    slimList.addString("hello");
    slimList.addString("goodbye");
    list.addString("hello");
    list.addString("goodbye");
    EXPECT_TRUE(slimList.equals(&list));
}

TEST_F(SlimListTest, twoNonIdenticalMultipleElementListsAreNotEqual)
{
    SlimList list;
    slimList.addString("hello");
    slimList.addString("hello");
    list.addString("hello");
    list.addString("goodbye");
    EXPECT_FALSE(slimList.equals(&list));
}

TEST_F(SlimListTest, canGetElements)
{
    slimList.addString("element1");
    slimList.addString("element2");
    EXPECT_STREQ("element1", slimList.getStringAt(0));
    EXPECT_STREQ("element2", slimList.getStringAt(1));
}

TEST_F(SlimListTest, canGetHashWithOneElement)
{
    slimList.addString("<table><tr><td>name</td><td>bob</td></tr></table>");
    auto hash            = slimList.getHashAt(0);
    SlimList* twoElementList = hash->getListAt(0);
    EXPECT_STREQ("name", twoElementList->getStringAt(0));
    EXPECT_STREQ("bob",  twoElementList->getStringAt(1));
}

TEST_F(SlimListTest, canGetHashWithMultipleElements)
{
    slimList.addString("<table><tr><td>name</td><td>dough</td></tr><tr><td>addr</td><td>here</td></tr></table>");
    auto hash            = slimList.getHashAt(0);
    SlimList* twoElementList = hash->getListAt(1);
    EXPECT_STREQ("addr", twoElementList->getStringAt(0));
    EXPECT_STREQ("here", twoElementList->getStringAt(1));
}

TEST_F(SlimListTest, cannotGetElementThatIsNotThere)
{
    slimList.addString("element1");
    slimList.addString("element2");
    EXPECT_EQ((const char*)nullptr, slimList.getStringAt(3));
}

TEST_F(SlimListTest, canReplaceString)
{
    slimList.addString("replaceMe");
    slimList.replaceAt(0, "WithMe");
    EXPECT_STREQ("WithMe", slimList.getStringAt(0));
}

TEST_F(SlimListTest, canGetTail)
{
    slimList.addString("1");
    slimList.addString("2");
    slimList.addString("3");
    slimList.addString("4");

    SlimList expected;
    expected.addString("3");
    expected.addString("4");

    auto tail = slimList.getTailAt(2);
    EXPECT_TRUE(expected.equals(tail.get()));
}

TEST_F(SlimListTest, getDouble)
{
    slimList.addString("2.3");
    EXPECT_NEAR(2.3, slimList.getDoubleAt(0), 0.1);
}

TEST_F(SlimListTest, ToStringForEmptyList)
{
    EXPECT_EQ("[]", slimList.toString());
}

TEST_F(SlimListTest, toStringForSimpleList)
{
    slimList.addString("a");
    slimList.addString("b");
    EXPECT_EQ("[\"a\", \"b\"]", slimList.toString());
}

TEST_F(SlimListTest, toStringDoesNotHaveASideEffectWhichChangesResultsFromPriorCalls)
{
    std::string priorString = slimList.toString();
    slimList.addString("a");
    std::string withElement = slimList.toString();
    EXPECT_NE(priorString, withElement);
}

TEST_F(SlimListTest, recursiveToString)
{
    slimList.addString("a");
    slimList.addString("b");
    SlimList sublist;
    sublist.addString("3");
    sublist.addString("4");
    slimList.addList(&sublist);
    EXPECT_EQ("[\"a\", \"b\", [\"3\", \"4\"]]", slimList.toString());
}

TEST_F(SlimListTest, toStringForLongList)
{
    for (int i = 0; i < 128; i++)
        slimList.addString("a");
    (void)slimList.toString();
}

TEST_F(SlimListTest, CanPopHeadOnListWithOneEntry)
{
    slimList.addString("a");
    slimList.popHead();
    EXPECT_EQ(0, slimList.getLength());
}

TEST_F(SlimListTest, CanInsertAfterPoppingListWithEntries)
{
    slimList.addString("a");
    slimList.addString("a");
    slimList.popHead();
    slimList.popHead();
    slimList.addString("a");
    slimList.addString("a");
    EXPECT_EQ(2, slimList.getLength());
}

TEST_F(SlimListTest, iteratorDoesNotHaveAnItemWhenEmpty)
{
    EXPECT_EQ(nullptr, slimList.createIterator());
}

TEST_F(SlimListTest, iteratorHasItem)
{
    slimList.addString("a");
    EXPECT_NE(nullptr, slimList.createIterator());
}

TEST_F(SlimListTest, iteratorNext)
{
    slimList.addString("a");
    SlimListIterator* it = slimList.createIterator();
    EXPECT_EQ(nullptr, it->advance());
}

TEST_F(SlimListTest, iteratorGetString)
{
    slimList.addString("a");
    SlimListIterator* it = slimList.createIterator();
    EXPECT_STREQ("a", it->getString());
}
