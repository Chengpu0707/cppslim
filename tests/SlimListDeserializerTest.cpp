#include <gtest/gtest.h>
#include "SlimList.h"
#include "SlimListDeserializer.h"
#include "SlimListSerializer.h"

class SlimListDeserializerTest : public ::testing::Test {
protected:
    SlimList* slimList;
    SlimList* deserializedList;
    char*     serializedList;

    void SetUp() override {
        slimList         = SlimList_Create();
        deserializedList = nullptr;
        serializedList   = nullptr;
    }
    void TearDown() override {
        SlimList_Destroy(slimList);
        if (deserializedList) SlimList_Destroy(deserializedList);
        if (serializedList)   SlimList_Release(serializedList);
    }
    void check_lists_equal(SlimList* expected, SlimList* actual) {
        EXPECT_TRUE(SlimList_Equals(expected, actual));
    }
};

TEST_F(SlimListDeserializerTest, deserializeEmptyList)
{
    deserializedList = SlimList_Deserialize("[000000:]");
    EXPECT_NE((SlimList*)nullptr, deserializedList);
    EXPECT_EQ(0, SlimList_GetLength(deserializedList));
}

TEST_F(SlimListDeserializerTest, deserializeNull)
{
    EXPECT_EQ((SlimList*)nullptr, SlimList_Deserialize(nullptr));
}

TEST_F(SlimListDeserializerTest, deserializeEmptyString)
{
    EXPECT_EQ((SlimList*)nullptr, SlimList_Deserialize(""));
}

TEST_F(SlimListDeserializerTest, MissingOpenBracketReturnsNull)
{
    EXPECT_EQ((SlimList*)nullptr, SlimList_Deserialize("hello"));
}

TEST_F(SlimListDeserializerTest, MissingClosingBracketReturnsNull)
{
    EXPECT_EQ((SlimList*)nullptr, SlimList_Deserialize("[000000:"));
}

TEST_F(SlimListDeserializerTest, canDeserializeCanonicalListWithOneElement)
{
    SlimList* list = SlimList_Deserialize("[000001:000008:Hi doug.:]");
    ASSERT_NE((SlimList*)nullptr, list);
    EXPECT_EQ(1, SlimList_GetLength(list));
    EXPECT_STREQ("Hi doug.", SlimList_GetStringAt(list, 0));
    SlimList_Destroy(list);
}

TEST_F(SlimListDeserializerTest, canDeserializeWithMultibyteCharacters)
{
    SlimList* list = SlimList_Deserialize("[000001:000008:Hi JRÜ€©:]");
    ASSERT_NE((SlimList*)nullptr, list);
    EXPECT_EQ(1, SlimList_GetLength(list));
    EXPECT_STREQ("Hi JRÜ€©", SlimList_GetStringAt(list, 0));
    SlimList_Destroy(list);
}

TEST_F(SlimListDeserializerTest, canDeSerializeListWithOneElement)
{
    SlimList_AddString(slimList, "hello");
    serializedList   = SlimList_Serialize(slimList);
    deserializedList = SlimList_Deserialize(serializedList);
    ASSERT_NE((SlimList*)nullptr, deserializedList);
    check_lists_equal(slimList, deserializedList);
}

TEST_F(SlimListDeserializerTest, canDeSerializeListWithTwoElements)
{
    SlimList_AddString(slimList, "hello");
    SlimList_AddString(slimList, "bob");
    serializedList   = SlimList_Serialize(slimList);
    deserializedList = SlimList_Deserialize(serializedList);
    ASSERT_NE((SlimList*)nullptr, deserializedList);
    check_lists_equal(slimList, deserializedList);
}

TEST_F(SlimListDeserializerTest, canAddSubList)
{
    SlimList* embedded = SlimList_Create();
    SlimList_AddString(embedded, "element");
    SlimList_AddList(slimList, embedded);
    serializedList   = SlimList_Serialize(slimList);
    deserializedList = SlimList_Deserialize(serializedList);
    SlimList* subList = SlimList_GetListAt(deserializedList, 0);
    check_lists_equal(embedded, subList);
    SlimList_Destroy(embedded);
}

TEST_F(SlimListDeserializerTest, getStringWhereThereIsAList)
{
    SlimList* embedded = SlimList_Create();
    SlimList_AddString(embedded, "element");
    SlimList_AddList(slimList, embedded);
    serializedList   = SlimList_Serialize(slimList);
    deserializedList = SlimList_Deserialize(serializedList);
    const char* string = SlimList_GetStringAt(deserializedList, 0);
    EXPECT_STREQ("[000001:000007:element:]", string);
    SlimList_Destroy(embedded);
}
