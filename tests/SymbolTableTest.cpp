#include <gtest/gtest.h>
#include "SymbolTable.h"

class SymbolTableTest : public ::testing::Test {
protected:
    SymbolTable* symbolTable;
    void SetUp() override    { symbolTable = SymbolTable_Create(); }
    void TearDown() override { SymbolTable_Destroy(symbolTable); }
};

TEST_F(SymbolTableTest, findNonExistentSymbolShouldReturnNull)
{
    EXPECT_EQ(nullptr, SymbolTable_FindSymbol(symbolTable, "Hey", 3));
}

TEST_F(SymbolTableTest, findSymbolShouldReturnSymbol)
{
    SymbolTable_SetSymbol(symbolTable, "Hey", "You");
    EXPECT_STREQ("You", SymbolTable_FindSymbol(symbolTable, "Hey", 3));
}

TEST_F(SymbolTableTest, CanGetLengthOfSymbol)
{
    SymbolTable_SetSymbol(symbolTable, "Hey", "1234567890");
    EXPECT_EQ(10, SymbolTable_GetSymbolLength(symbolTable, "Hey", 3));
}

TEST_F(SymbolTableTest, CanGetLengthOfNonExistentSymbol)
{
    EXPECT_EQ(-1, SymbolTable_GetSymbolLength(symbolTable, "Hey", 3));
}
