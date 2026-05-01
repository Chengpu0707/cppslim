#include "SymbolTable.h"
#include <cstring>

const char* SymbolTable::findSymbol(const char* name, int length) const
{
    auto it = map_.find(std::string(name, static_cast<std::size_t>(length)));
    return it != map_.end() ? it->second.c_str() : nullptr;
}

void SymbolTable::setSymbol(const char* symbol, const char* value)
{
    map_[symbol] = value ? value : "";
}

int SymbolTable::getSymbolLength(const char* symbol, int length) const
{
    const char* v = findSymbol(symbol, length);
    return v ? static_cast<int>(std::strlen(v)) : -1;
}
