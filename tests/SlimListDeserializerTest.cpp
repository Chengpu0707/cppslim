#include <gtest/gtest.h>
#include "SlimList.h"

class SlimListDeserializerTest : public ::testing::Test {
protected:
    SlimList                  slimList;
    std::unique_ptr<SlimList> deserializedList;
    std::string               serializedList;

    void check_lists_equal(const SlimList* expected, const SlimList* actual) {
        EXPECT_TRUE(expected->equals(actual));
    }
};

TEST_F(SlimListDeserializerTest, deserializeEmptyList)
{
    deserializedList = SlimList::deserialize("[000000:]");
    ASSERT_TRUE(deserializedList);
    EXPECT_EQ(0, deserializedList->getLength());
}

TEST_F(SlimListDeserializerTest, deserializeNull)
{
    EXPECT_FALSE(SlimList::deserialize(nullptr));
}

TEST_F(SlimListDeserializerTest, deserializeEmptyString)
{
    EXPECT_FALSE(SlimList::deserialize(""));
}

TEST_F(SlimListDeserializerTest, MissingOpenBracketReturnsNull)
{
    EXPECT_FALSE(SlimList::deserialize("hello"));
}

TEST_F(SlimListDeserializerTest, MissingClosingBracketReturnsNull)
{
    EXPECT_FALSE(SlimList::deserialize("[000000:"));
}

TEST_F(SlimListDeserializerTest, canDeserializeCanonicalListWithOneElement)
{
    auto list = SlimList::deserialize("[000001:000008:Hi doug.:]");
    ASSERT_TRUE(list);
    EXPECT_EQ(1, list->getLength());
    EXPECT_STREQ("Hi doug.", list->getStringAt(0));
}

TEST_F(SlimListDeserializerTest, canDeserializeWithMultibyteCharacters)
{
    auto list = SlimList::deserialize("[000001:000008:Hi JRÜ€©:]");
    ASSERT_TRUE(list);
    EXPECT_EQ(1, list->getLength());
    EXPECT_STREQ("Hi JRÜ€©", list->getStringAt(0));
}

TEST_F(SlimListDeserializerTest, canDeSerializeListWithOneElement)
{
    slimList.addString("hello");
    serializedList   = slimList.serialize();
    deserializedList = SlimList::deserialize(serializedList.c_str());
    ASSERT_TRUE(deserializedList);
    check_lists_equal(&slimList, deserializedList.get());
}

TEST_F(SlimListDeserializerTest, canDeSerializeListWithTwoElements)
{
    slimList.addString("hello");
    slimList.addString("bob");
    serializedList   = slimList.serialize();
    deserializedList = SlimList::deserialize(serializedList.c_str());
    ASSERT_TRUE(deserializedList);
    check_lists_equal(&slimList, deserializedList.get());
}

TEST_F(SlimListDeserializerTest, canAddSubList)
{
    SlimList embedded;
    embedded.addString("element");
    slimList.addList(&embedded);
    serializedList   = slimList.serialize();
    deserializedList = SlimList::deserialize(serializedList.c_str());
    SlimList* subList = deserializedList->getListAt(0);
    check_lists_equal(&embedded, subList);
}

TEST_F(SlimListDeserializerTest, getStringWhereThereIsAList)
{
    SlimList embedded;
    embedded.addString("element");
    slimList.addList(&embedded);
    serializedList   = slimList.serialize();
    deserializedList = SlimList::deserialize(serializedList.c_str());
    const char* s = deserializedList->getStringAt(0);
    EXPECT_STREQ("[000001:000007:element:]", s);
}
