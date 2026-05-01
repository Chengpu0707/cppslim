#include <gtest/gtest.h>
#include <string.h>
#include "SlimList.h"

class SlimListSerializerTest : public ::testing::Test {
protected:
    SlimList* slimList;
    char*     serializedList;

    void SetUp() override {
        slimList       = new SlimList();
        serializedList = nullptr;
    }
    void TearDown() override {
        delete slimList;
        SlimList::release(serializedList);
    }
};

TEST_F(SlimListSerializerTest, SerializeAListWithNoElements)
{
    serializedList = slimList->serialize();
    EXPECT_STREQ("[000000:]", serializedList);
}

TEST_F(SlimListSerializerTest, SerializeAListWithOneElement)
{
    slimList->addString("hello");
    serializedList = slimList->serialize();
    EXPECT_STREQ("[000001:000005:hello:]", serializedList);
}

TEST_F(SlimListSerializerTest, SerializeAListWithTwoElements)
{
    slimList->addString("hello");
    slimList->addString("world");
    serializedList = slimList->serialize();
    EXPECT_STREQ("[000002:000005:hello:000005:world:]", serializedList);
}

TEST_F(SlimListSerializerTest, ListCopiesItsString)
{
    char string[12] = "Hello";
    slimList->addString(string);
    strcpy(string, "Goodbye");
    serializedList = slimList->serialize();
    EXPECT_STREQ("[000001:000005:Hello:]", serializedList);
}

TEST_F(SlimListSerializerTest, canCopyAList)
{
    slimList->addString("123456");
    slimList->addString("987654");
    SlimList* copy = new SlimList();
    for (int i = 0; i < slimList->getLength(); i++)
        copy->addString(slimList->getStringAt(i));
    char* serialCopy = copy->serialize();
    char* serialOrig = slimList->serialize();
    EXPECT_STREQ(serialCopy, serialOrig);
    delete copy;
    SlimList::release(serialOrig);
    SlimList::release(serialCopy);
}

TEST_F(SlimListSerializerTest, SerializeNestedList)
{
    SlimList* embedded = new SlimList();
    embedded->addString("element");
    slimList->addList(embedded);
    serializedList = slimList->serialize();
    EXPECT_STREQ("[000001:000024:[000001:000007:element:]:]", serializedList);
    delete embedded;
}

TEST_F(SlimListSerializerTest, serializedLength)
{
    slimList->addString("12345");
    EXPECT_EQ(5 + 17, slimList->serializedLength());
    slimList->addString("123456");
    EXPECT_EQ(9 + (5 + 8) + (6 + 8), slimList->serializedLength());
    serializedList = slimList->serialize();
    EXPECT_EQ(9 + (5 + 8) + (6 + 8), (int)strlen(serializedList));
}

TEST_F(SlimListSerializerTest, serializeNull)
{
    slimList->addString(nullptr);
    serializedList = slimList->serialize();
    EXPECT_STREQ("[000001:000004:null:]", serializedList);
}

TEST_F(SlimListSerializerTest, serializeMultibyteCharacters)
{
    slimList->addString("Ü€©phewÜ€©");
    serializedList = slimList->serialize();
    EXPECT_STREQ("[000001:000010:Ü€©phewÜ€©:]", serializedList);
}
