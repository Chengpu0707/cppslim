#pragma once
#include "SlimList.h"
#include <memory>
#include <string>
#include <vector>
#include <deque>

class SymbolTable;
class StatementExecutor;

typedef void        (*Fixture)(StatementExecutor*);
typedef void*       (*Constructor)(StatementExecutor*, SlimList*);
typedef void        (*Destructor)(void*);
typedef const char* (*Method)(void*, SlimList*);

class StatementExecutor {
public:
    StatementExecutor();
    ~StatementExecutor();

    void        addFixture(Fixture);
    void        registerFixture(const char* className, Constructor, Destructor);
    void        registerMethod(const char* className, const char* methodName, Method);

    const char* make(const char* instanceName, const char* className, SlimList* args);
    const char* call(const char* instanceName, const char* methodName, SlimList* args);
    void*       instance(const char* instanceName);
    void        setSymbol(const char* symbol, const char* value);

    void               constructorError(const char* message);
    static const char* fixtureError(const char* message);

private:
    struct MethodInfo {
        std::string name;
        Method      method;
    };
    struct FixtureInfo {
        std::string             name;
        Constructor             constructor = nullptr;
        Destructor              destructor  = nullptr;
        std::vector<MethodInfo> methods;
    };
    struct InstanceInfo {
        std::string  name;
        void*        instance;
        FixtureInfo* fixture;   // stable pointer into fixtures_ deque
    };

    std::deque<FixtureInfo>  fixtures_;
    std::deque<InstanceInfo> instances_;        // front = most recently created (LIFO)
    std::deque<InstanceInfo> libraryInstances_;
    std::unique_ptr<SymbolTable> symbolTable_;
    std::string              message_;
    std::string              userMessage_;

    FixtureInfo*  findFixtureByName(const char* className);
    FixtureInfo*  findFixture(const char* classNameWithSymbols);
    InstanceInfo* getInstanceNode(const char* instanceName);
    const char*   invokeMethod(MethodInfo* method, InstanceInfo* inst, SlimList* args);
    void          replaceSymbols(SlimList* list);
    std::string   replaceString(const char* s);
    std::string   replaceStringFrom(const std::string& str, std::size_t from);
};
