#pragma once
#include <string>
#include <unordered_map>

class SymbolTable {
public:
    const char* findSymbol(const char* name, int length) const;
    void        setSymbol(const char* symbol, const char* value);
    int         getSymbolLength(const char* symbol, int length) const;
private:
    std::unordered_map<std::string, std::string> map_;
};
