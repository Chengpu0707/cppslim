#include <gtest/gtest.h>
#include <string.h>
#include "SlimList.h"
#include "SlimListSerializer.h"

class SlimListSerializerTest : public ::testing::Test {
protected:
    SlimList* slimList;
    char*     serializedList;

    void SetUp() override {
        slimList       = SlimList_Create();
        serializedList = nullptr;
    }
    void TearDown() override {
        SlimList_Destroy(slimList);
        SlimList_Release(serializedList);
    }
};

TEST_F(SlimListSerializerTest, SerializeAListWithNoElements)
{
    serializedList = SlimList_Serialize(slimList);
    EXPECT_STREQ("[000000:]", serializedList);
}

TEST_F(SlimListSerializerTest, SerializeAListWithOneElement)
{
    SlimList_AddString(slimList, "hello");
    serializedList = SlimList_Serialize(slimList);
    EXPECT_STREQ("[000001:000005:hello:]", serializedList);
}

TEST_F(SlimListSerializerTest, SerializeAListWithTwoElements)
{
    SlimList_AddString(slimList, "hello");
    SlimList_AddString(slimList, "world");
    serializedList = SlimList_Serialize(slimList);
    EXPECT_STREQ("[000002:000005:hello:000005:world:]", serializedList);
}

TEST_F(SlimListSerializerTest, ListCopiesItsString)
{
    char string[12] = "Hello";
    SlimList_AddString(slimList, string);
    strcpy(string, "Goodbye");
    serializedList = SlimList_Serialize(slimList);
    EXPECT_STREQ("[000001:000005:Hello:]", serializedList);
}

TEST_F(SlimListSerializerTest, canCopyAList)
{
    SlimList_AddString(slimList, "123456");
    SlimList_AddString(slimList, "987654");
    SlimList* copy = SlimList_Create();
    for (int i = 0; i < SlimList_GetLength(slimList); i++)
        SlimList_AddString(copy, SlimList_GetStringAt(slimList, i));
    char* serialCopy = SlimList_Serialize(copy);
    char* serialOrig = SlimList_Serialize(slimList);
    EXPECT_STREQ(serialCopy, serialOrig);
    SlimList_Destroy(copy);
    SlimList_Release(serialOrig);
    SlimList_Release(serialCopy);
}

TEST_F(SlimListSerializerTest, SerializeNestedList)
{
    SlimList* embedded = SlimList_Create();
    SlimList_AddString(embedded, "element");
    SlimList_AddList(slimList, embedded);
    serializedList = SlimList_Serialize(slimList);
    EXPECT_STREQ("[000001:000024:[000001:000007:element:]:]", serializedList);
    SlimList_Destroy(embedded);
}

TEST_F(SlimListSerializerTest, serializedLength)
{
    SlimList_AddString(slimList, "12345");
    EXPECT_EQ(5 + 17, SlimList_SerializedLength(slimList));
    SlimList_AddString(slimList, "123456");
    EXPECT_EQ(9 + (5 + 8) + (6 + 8), SlimList_SerializedLength(slimList));
    serializedList = SlimList_Serialize(slimList);
    EXPECT_EQ(9 + (5 + 8) + (6 + 8), (int)strlen(serializedList));
}

TEST_F(SlimListSerializerTest, serializeNull)
{
    SlimList_AddString(slimList, NULL);
    serializedList = SlimList_Serialize(slimList);
    EXPECT_STREQ("[000001:000004:null:]", serializedList);
}

TEST_F(SlimListSerializerTest, serializeMultibyteCharacters)
{
    SlimList_AddString(slimList, "Ü€©phewÜ€©");
    serializedList = SlimList_Serialize(slimList);
    EXPECT_STREQ("[000001:000010:Ü€©phewÜ€©:]", serializedList);
}
