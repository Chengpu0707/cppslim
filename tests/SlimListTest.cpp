#include <gtest/gtest.h>
#include <string.h>
#include "SlimList.h"
#include "SlimUtil.h"

class SlimListTest : public ::testing::Test {
protected:
    SlimList* slimList;
    SlimList* deserializedList;

    void SetUp() override {
        slimList = SlimList_Create();
        deserializedList = nullptr;
    }
    void TearDown() override {
        SlimList_Destroy(slimList);
        if (deserializedList)
            SlimList_Destroy(deserializedList);
    }
    void check_lists_equal(SlimList* expected, SlimList* actual) {
        EXPECT_TRUE(SlimList_Equals(expected, actual));
    }
};

TEST_F(SlimListTest, twoEmptyListsAreEqual)
{
    SlimList* list = SlimList_Create();
    check_lists_equal(slimList, list);
    SlimList_Destroy(list);
}

TEST_F(SlimListTest, twoDifferentLengthListsAreNotEqual)
{
    SlimList* list = SlimList_Create();
    SlimList_AddString(slimList, "hello");
    EXPECT_FALSE(SlimList_Equals(slimList, list));
    SlimList_Destroy(list);
}

TEST_F(SlimListTest, twoSingleElementListsWithDifferentElementsAreNotEqual)
{
    SlimList* list = SlimList_Create();
    SlimList_AddString(slimList, "hello");
    SlimList_AddString(list, "goodbye");
    EXPECT_FALSE(SlimList_Equals(slimList, list));
    SlimList_Destroy(list);
}

TEST_F(SlimListTest, twoIdenticalMultipleElementListsAreEqual)
{
    SlimList* list = SlimList_Create();
    SlimList_AddString(slimList, "hello");
    SlimList_AddString(slimList, "goodbye");
    SlimList_AddString(list, "hello");
    SlimList_AddString(list, "goodbye");
    EXPECT_TRUE(SlimList_Equals(slimList, list));
    SlimList_Destroy(list);
}

TEST_F(SlimListTest, twoNonIdenticalMultipleElementListsAreNotEqual)
{
    SlimList* list = SlimList_Create();
    SlimList_AddString(slimList, "hello");
    SlimList_AddString(slimList, "hello");
    SlimList_AddString(list, "hello");
    SlimList_AddString(list, "goodbye");
    EXPECT_FALSE(SlimList_Equals(slimList, list));
    SlimList_Destroy(list);
}

TEST_F(SlimListTest, canGetElements)
{
    SlimList_AddString(slimList, "element1");
    SlimList_AddString(slimList, "element2");
    EXPECT_STREQ("element1", SlimList_GetStringAt(slimList, 0));
    EXPECT_STREQ("element2", SlimList_GetStringAt(slimList, 1));
}

TEST_F(SlimListTest, canGetHashWithOneElement)
{
    SlimList_AddString(slimList, "<table><tr><td>name</td><td>bob</td></tr></table>");
    SlimList* hash = SlimList_GetHashAt(slimList, 0);
    SlimList* twoElementList = SlimList_GetListAt(hash, 0);
    EXPECT_STREQ("name", SlimList_GetStringAt(twoElementList, 0));
    EXPECT_STREQ("bob",  SlimList_GetStringAt(twoElementList, 1));
    SlimList_Destroy(hash);
}

TEST_F(SlimListTest, canGetHashWithMultipleElements)
{
    SlimList_AddString(slimList, "<table><tr><td>name</td><td>dough</td></tr><tr><td>addr</td><td>here</td></tr></table>");
    SlimList* hash = SlimList_GetHashAt(slimList, 0);
    SlimList* twoElementList = SlimList_GetListAt(hash, 1);
    EXPECT_STREQ("addr", SlimList_GetStringAt(twoElementList, 0));
    EXPECT_STREQ("here", SlimList_GetStringAt(twoElementList, 1));
    SlimList_Destroy(hash);
}

TEST_F(SlimListTest, cannotGetElementThatIsNotThere)
{
    SlimList_AddString(slimList, "element1");
    SlimList_AddString(slimList, "element2");
    EXPECT_EQ((const char*)0, SlimList_GetStringAt(slimList, 3));
}

TEST_F(SlimListTest, canReplaceString)
{
    SlimList_AddString(slimList, "replaceMe");
    SlimList_ReplaceAt(slimList, 0, "WithMe");
    EXPECT_STREQ("WithMe", SlimList_GetStringAt(slimList, 0));
}

TEST_F(SlimListTest, canGetTail)
{
    SlimList_AddString(slimList, "1");
    SlimList_AddString(slimList, "2");
    SlimList_AddString(slimList, "3");
    SlimList_AddString(slimList, "4");

    SlimList* expected = SlimList_Create();
    SlimList_AddString(expected, "3");
    SlimList_AddString(expected, "4");

    SlimList* tail = SlimList_GetTailAt(slimList, 2);
    EXPECT_TRUE(SlimList_Equals(expected, tail));
    SlimList_Destroy(tail);
    SlimList_Destroy(expected);
}

TEST_F(SlimListTest, getDouble)
{
    SlimList_AddString(slimList, "2.3");
    EXPECT_NEAR(2.3, SlimList_GetDoubleAt(slimList, 0), 0.1);
}

TEST_F(SlimListTest, ToStringForEmptyList)
{
    const char* s = SlimList_ToString(slimList);
    EXPECT_STREQ("[]", s);
    CSlim_DestroyString(s);
}

TEST_F(SlimListTest, toStringForSimpleList)
{
    SlimList_AddString(slimList, "a");
    SlimList_AddString(slimList, "b");
    const char* s = SlimList_ToString(slimList);
    EXPECT_STREQ("[\"a\", \"b\"]", s);
    CSlim_DestroyString(s);
}

TEST_F(SlimListTest, toStringDoesNotHaveASideEffectWhichChangesResultsFromPriorCalls)
{
    const char* priorString = SlimList_ToString(slimList);
    SlimList_AddString(slimList, "a");
    const char* withElement = SlimList_ToString(slimList);
    EXPECT_FALSE(strcmp(priorString, withElement) == 0);
    CSlim_DestroyString(priorString);
    CSlim_DestroyString(withElement);
}

TEST_F(SlimListTest, recursiveToString)
{
    SlimList_AddString(slimList, "a");
    SlimList_AddString(slimList, "b");
    SlimList* sublist = SlimList_Create();
    SlimList_AddString(sublist, "3");
    SlimList_AddString(sublist, "4");
    SlimList_AddList(slimList, sublist);
    const char* s = SlimList_ToString(slimList);
    EXPECT_STREQ("[\"a\", \"b\", [\"3\", \"4\"]]", s);
    CSlim_DestroyString(s);
    SlimList_Destroy(sublist);
}

TEST_F(SlimListTest, toStringForLongList)
{
    for (int i = 0; i < 128; i++)
        SlimList_AddString(slimList, "a");
    const char* s = SlimList_ToString(slimList);
    CSlim_DestroyString(s);
}

TEST_F(SlimListTest, CanPopHeadOnListWithOneEntry)
{
    SlimList_AddString(slimList, "a");
    SlimList_PopHead(slimList);
    EXPECT_EQ(0, SlimList_GetLength(slimList));
}

TEST_F(SlimListTest, CanInsertAfterPoppingListWithEntries)
{
    SlimList_AddString(slimList, "a");
    SlimList_AddString(slimList, "a");
    SlimList_PopHead(slimList);
    SlimList_PopHead(slimList);
    SlimList_AddString(slimList, "a");
    SlimList_AddString(slimList, "a");
    EXPECT_EQ(2, SlimList_GetLength(slimList));
}

TEST_F(SlimListTest, iteratorDoesNotHaveAnItemWhenEmpty)
{
    SlimListIterator* it = SlimList_CreateIterator(slimList);
    EXPECT_FALSE(SlimList_Iterator_HasItem(it));
}

TEST_F(SlimListTest, iteratorHasItem)
{
    SlimList_AddString(slimList, "a");
    SlimListIterator* it = SlimList_CreateIterator(slimList);
    EXPECT_TRUE(SlimList_Iterator_HasItem(it));
}

TEST_F(SlimListTest, iteratorNext)
{
    SlimList_AddString(slimList, "a");
    SlimListIterator* it = SlimList_CreateIterator(slimList);
    SlimList_Iterator_Advance(&it);
    EXPECT_FALSE(SlimList_Iterator_HasItem(it));
}

TEST_F(SlimListTest, iteratorGetString)
{
    const char* contents = "a";
    SlimList_AddString(slimList, contents);
    SlimListIterator* it = SlimList_CreateIterator(slimList);
    EXPECT_STREQ(contents, SlimList_Iterator_GetString(it));
}
