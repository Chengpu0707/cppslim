#include <gtest/gtest.h>
#include <string.h>
#include "SlimList.h"

class SlimListSerializerTest : public ::testing::Test {
protected:
    SlimList    slimList;
    std::string serializedList;
};

TEST_F(SlimListSerializerTest, SerializeAListWithNoElements)
{
    serializedList = slimList.serialize();
    EXPECT_EQ("[000000:]", serializedList);
}

TEST_F(SlimListSerializerTest, SerializeAListWithOneElement)
{
    slimList.addString("hello");
    serializedList = slimList.serialize();
    EXPECT_EQ("[000001:000005:hello:]", serializedList);
}

TEST_F(SlimListSerializerTest, SerializeAListWithTwoElements)
{
    slimList.addString("hello");
    slimList.addString("world");
    serializedList = slimList.serialize();
    EXPECT_EQ("[000002:000005:hello:000005:world:]", serializedList);
}

TEST_F(SlimListSerializerTest, ListCopiesItsString)
{
    char string[12] = "Hello";
    slimList.addString(string);
    strcpy(string, "Goodbye");
    serializedList = slimList.serialize();
    EXPECT_EQ("[000001:000005:Hello:]", serializedList);
}

TEST_F(SlimListSerializerTest, canCopyAList)
{
    slimList.addString("123456");
    slimList.addString("987654");
    SlimList copy;
    for (int i = 0; i < slimList.getLength(); i++)
        copy.addString(slimList.getStringAt(i));
    EXPECT_EQ(copy.serialize(), slimList.serialize());
}

TEST_F(SlimListSerializerTest, SerializeNestedList)
{
    SlimList embedded;
    embedded.addString("element");
    slimList.addList(&embedded);
    serializedList = slimList.serialize();
    EXPECT_EQ("[000001:000024:[000001:000007:element:]:]", serializedList);
}

TEST_F(SlimListSerializerTest, serializedLength)
{
    slimList.addString("12345");
    EXPECT_EQ(5 + 17, slimList.serializedLength());
    slimList.addString("123456");
    EXPECT_EQ(9 + (5 + 8) + (6 + 8), slimList.serializedLength());
    serializedList = slimList.serialize();
    EXPECT_EQ(9 + (5 + 8) + (6 + 8), (int)serializedList.size());
}

TEST_F(SlimListSerializerTest, serializeNull)
{
    slimList.addString(nullptr);
    serializedList = slimList.serialize();
    EXPECT_EQ("[000001:000004:null:]", serializedList);
}

TEST_F(SlimListSerializerTest, serializeMultibyteCharacters)
{
    slimList.addString("Ü€©phewÜ€©");
    serializedList = slimList.serialize();
    EXPECT_EQ("[000001:000010:Ü€©phewÜ€©:]", serializedList);
}
