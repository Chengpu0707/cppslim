#include "SymbolTable.h"
#include <string>
#include <unordered_map>
#include <cstring>

struct SymbolTable {
    std::unordered_map<std::string, std::string> map;
};

SymbolTable* SymbolTable_Create()
{
    return new SymbolTable();
}

void SymbolTable_Destroy(SymbolTable* self)
{
    delete self;
}

const char* SymbolTable_FindSymbol(SymbolTable* self, char const* name, int length)
{
    auto it = self->map.find(std::string(name, static_cast<std::size_t>(length)));
    return it != self->map.end() ? it->second.c_str() : nullptr;
}

void SymbolTable_SetSymbol(SymbolTable* self, char const* symbol, char const* value)
{
    self->map[symbol] = value ? value : "";
}

int SymbolTable_GetSymbolLength(SymbolTable* self, char const* symbol, int length)
{
    const char* v = SymbolTable_FindSymbol(self, symbol, length);
    return v ? static_cast<int>(std::strlen(v)) : -1;
}
