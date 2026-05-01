#include <gtest/gtest.h>
#include "SymbolTable.h"

class SymbolTableTest : public ::testing::Test {
protected:
    SymbolTable* symbolTable;
    void SetUp()    override { symbolTable = new SymbolTable(); }
    void TearDown() override { delete symbolTable; }
};

TEST_F(SymbolTableTest, findNonExistentSymbolShouldReturnNull)
{
    EXPECT_EQ(nullptr, symbolTable->findSymbol("Hey", 3));
}

TEST_F(SymbolTableTest, findSymbolShouldReturnSymbol)
{
    symbolTable->setSymbol("Hey", "You");
    EXPECT_STREQ("You", symbolTable->findSymbol("Hey", 3));
}

TEST_F(SymbolTableTest, CanGetLengthOfSymbol)
{
    symbolTable->setSymbol("Hey", "1234567890");
    EXPECT_EQ(10, symbolTable->getSymbolLength("Hey", 3));
}

TEST_F(SymbolTableTest, CanGetLengthOfNonExistentSymbol)
{
    EXPECT_EQ(-1, symbolTable->getSymbolLength("Hey", 3));
}
