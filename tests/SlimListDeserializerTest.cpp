#include <gtest/gtest.h>
#include "SlimList.h"

class SlimListDeserializerTest : public ::testing::Test {
protected:
    SlimList* slimList;
    SlimList* deserializedList;
    char*     serializedList;

    void SetUp() override {
        slimList         = new SlimList();
        deserializedList = nullptr;
        serializedList   = nullptr;
    }
    void TearDown() override {
        delete slimList;
        delete deserializedList;
        SlimList::release(serializedList);
    }
    void check_lists_equal(SlimList* expected, SlimList* actual) {
        EXPECT_TRUE(expected->equals(actual));
    }
};

TEST_F(SlimListDeserializerTest, deserializeEmptyList)
{
    deserializedList = SlimList::deserialize("[000000:]");
    EXPECT_NE((SlimList*)nullptr, deserializedList);
    EXPECT_EQ(0, deserializedList->getLength());
}

TEST_F(SlimListDeserializerTest, deserializeNull)
{
    EXPECT_EQ((SlimList*)nullptr, SlimList::deserialize(nullptr));
}

TEST_F(SlimListDeserializerTest, deserializeEmptyString)
{
    EXPECT_EQ((SlimList*)nullptr, SlimList::deserialize(""));
}

TEST_F(SlimListDeserializerTest, MissingOpenBracketReturnsNull)
{
    EXPECT_EQ((SlimList*)nullptr, SlimList::deserialize("hello"));
}

TEST_F(SlimListDeserializerTest, MissingClosingBracketReturnsNull)
{
    EXPECT_EQ((SlimList*)nullptr, SlimList::deserialize("[000000:"));
}

TEST_F(SlimListDeserializerTest, canDeserializeCanonicalListWithOneElement)
{
    SlimList* list = SlimList::deserialize("[000001:000008:Hi doug.:]");
    ASSERT_NE((SlimList*)nullptr, list);
    EXPECT_EQ(1, list->getLength());
    EXPECT_STREQ("Hi doug.", list->getStringAt(0));
    delete list;
}

TEST_F(SlimListDeserializerTest, canDeserializeWithMultibyteCharacters)
{
    SlimList* list = SlimList::deserialize("[000001:000008:Hi JRÜ€©:]");
    ASSERT_NE((SlimList*)nullptr, list);
    EXPECT_EQ(1, list->getLength());
    EXPECT_STREQ("Hi JRÜ€©", list->getStringAt(0));
    delete list;
}

TEST_F(SlimListDeserializerTest, canDeSerializeListWithOneElement)
{
    slimList->addString("hello");
    serializedList   = slimList->serialize();
    deserializedList = SlimList::deserialize(serializedList);
    ASSERT_NE((SlimList*)nullptr, deserializedList);
    check_lists_equal(slimList, deserializedList);
}

TEST_F(SlimListDeserializerTest, canDeSerializeListWithTwoElements)
{
    slimList->addString("hello");
    slimList->addString("bob");
    serializedList   = slimList->serialize();
    deserializedList = SlimList::deserialize(serializedList);
    ASSERT_NE((SlimList*)nullptr, deserializedList);
    check_lists_equal(slimList, deserializedList);
}

TEST_F(SlimListDeserializerTest, canAddSubList)
{
    SlimList* embedded = new SlimList();
    embedded->addString("element");
    slimList->addList(embedded);
    serializedList   = slimList->serialize();
    deserializedList = SlimList::deserialize(serializedList);
    SlimList* subList = deserializedList->getListAt(0);
    check_lists_equal(embedded, subList);
    delete embedded;
}

TEST_F(SlimListDeserializerTest, getStringWhereThereIsAList)
{
    SlimList* embedded = new SlimList();
    embedded->addString("element");
    slimList->addList(embedded);
    serializedList   = slimList->serialize();
    deserializedList = SlimList::deserialize(serializedList);
    const char* s = deserializedList->getStringAt(0);
    EXPECT_STREQ("[000001:000007:element:]", s);
    delete embedded;
}
