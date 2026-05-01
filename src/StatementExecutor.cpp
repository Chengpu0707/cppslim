#include "StatementExecutor.h"
#include "SlimList.h"
#include "SlimListDeserializer.h"
#include "SlimListSerializer.h"
#include "SymbolTable.h"
#include <string>
#include <vector>
#include <deque>
#include <cstring>
#include <cctype>
#include <cassert>

struct MethodInfo {
    std::string name;
    Method method;
};

struct FixtureInfo {
    std::string name;
    Constructor constructor = nullptr;
    Destructor destructor = nullptr;
    std::vector<MethodInfo> methods;
};

struct InstanceInfo {
    std::string name;
    void* instance;
    FixtureInfo* fixture;  // stable pointer into StatementExecutor::fixtures deque
};

struct StatementExecutor {
    std::deque<FixtureInfo>  fixtures;
    std::deque<InstanceInfo> instances;         // front = most recently created (LIFO)
    std::deque<InstanceInfo> libraryInstances;
    SymbolTable* symbolTable;
    std::string message;
    std::string userMessage;
};

static std::string trunc32(const char* s)
{
    if (!s) return "";
    std::size_t len = std::strlen(s);
    return std::string(s, len > 32 ? 32 : len);
}

static bool compareNamesIgnoreUnderScores(const char* name1, const char* name2)
{
    while (*name1 && *name2) {
        if (*name1 == *name2) { name1++; name2++; }
        else if (*name1 == '_') name1++;
        else if (*name2 == '_') name2++;
        else return false;
    }
    return *name1 == *name2;
}

static FixtureInfo* findFixtureByName(StatementExecutor* executor, const char* className);
static FixtureInfo* findFixture(StatementExecutor* executor, const char* classNameWithSymbols);
static InstanceInfo* GetInstanceNode(StatementExecutor* executor, const char* instanceName);
static MethodInfo* findMethodNode(std::vector<MethodInfo>& methods, const char* methodName);
static const char* invokeMethod(StatementExecutor* executor, MethodInfo* method, InstanceInfo* inst, SlimList* args);
static void replaceSymbols(SymbolTable*, SlimList*);
static std::string replaceString(SymbolTable*, const char*);
static std::string replaceStringFrom(SymbolTable*, const std::string&, std::size_t from);
static bool isLibraryInstanceName(const char* instanceName);
static void* Null_Create(StatementExecutor*, SlimList*) { return nullptr; }
static void  Null_Destroy(void*) {}

StatementExecutor* StatementExecutor_Create()
{
    StatementExecutor* self = new StatementExecutor();
    self->symbolTable = SymbolTable_Create();
    return self;
}

void StatementExecutor_Destroy(StatementExecutor* self)
{
    for (auto& inst : self->libraryInstances)
        inst.fixture->destructor(inst.instance);
    for (auto& inst : self->instances)
        inst.fixture->destructor(inst.instance);
    SymbolTable_Destroy(self->symbolTable);
    delete self;
}

static InstanceInfo* GetInstanceNode(StatementExecutor* executor, const char* instanceName)
{
    for (auto& inst : executor->instances)
        if (compareNamesIgnoreUnderScores(inst.name.c_str(), instanceName))
            return &inst;
    return nullptr;
}

const char* StatementExecutor_Make(StatementExecutor* executor, char const* instanceName, char const* className, SlimList* args)
{
    FixtureInfo* fixture = findFixture(executor, className);
    if (!fixture) {
        executor->message = std::string("__EXCEPTION__:message:<<NO_CLASS ") + trunc32(className) + ".>>";
        return executor->message.c_str();
    }
    replaceSymbols(executor->symbolTable, args);
    executor->userMessage.clear();
    void* instance = fixture->constructor(executor, args);

    InstanceInfo inst{instanceName, instance, fixture};
    if (isLibraryInstanceName(instanceName))
        executor->libraryInstances.push_front(std::move(inst));
    else
        executor->instances.push_front(std::move(inst));

    if (instance != nullptr)
        return "OK";
    executor->message = std::string("__EXCEPTION__:message:<<COULD_NOT_INVOKE_CONSTRUCTOR ")
        + trunc32(className) + " " + trunc32(executor->userMessage.c_str()) + ".>>";
    return executor->message.c_str();
}

const char* StatementExecutor_Call(StatementExecutor* executor, char const* instanceName, char const* methodName, SlimList* args)
{
    InstanceInfo* inst = GetInstanceNode(executor, instanceName);
    if (inst) {
        MethodInfo* method = findMethodNode(inst->fixture->methods, methodName);
        if (method)
            return invokeMethod(executor, method, inst, args);

        for (auto& lib : executor->libraryInstances) {
            method = findMethodNode(lib.fixture->methods, methodName);
            if (method)
                return invokeMethod(executor, method, &lib, args);
        }

        executor->message = std::string("__EXCEPTION__:message:<<NO_METHOD_IN_CLASS ")
            + trunc32(methodName) + "[" + std::to_string(SlimList_GetLength(args)) + "] "
            + trunc32(inst->fixture->name.c_str()) + ".>>";
        return executor->message.c_str();
    }
    executor->message = std::string("__EXCEPTION__:message:<<NO_INSTANCE ")
        + trunc32(instanceName) + ".>>";
    return executor->message.c_str();
}

static FixtureInfo* findFixture(StatementExecutor* executor, const char* classNameWithSymbols)
{
    std::string className = replaceString(executor->symbolTable, classNameWithSymbols);
    return findFixtureByName(executor, className.c_str());
}

static bool isLibraryInstanceName(const char* instanceName)
{
    return strncmp(instanceName, "library", 7) == 0;
}

static MethodInfo* findMethodNode(std::vector<MethodInfo>& methods, const char* methodName)
{
    for (auto& m : methods)
        if (compareNamesIgnoreUnderScores(methodName, m.name.c_str()))
            return &m;
    return nullptr;
}

static const char* invokeMethod(StatementExecutor* executor, MethodInfo* method, InstanceInfo* inst, SlimList* args)
{
    replaceSymbols(executor->symbolTable, args);
    return method->method(inst->instance, args);
}

static void replaceSymbols(SymbolTable* symbolTable, SlimList* list)
{
    SlimListIterator* it = SlimList_CreateIterator(list);
    while (SlimList_Iterator_HasItem(it)) {
        const char* string = SlimList_Iterator_GetString(it);
        if (string != nullptr) {
            SlimList* embedded = SlimList_Deserialize(string);
            if (!embedded) {
                std::string replaced = replaceString(symbolTable, string);
                SlimList_Iterator_Replace(it, replaced.c_str());
            } else {
                replaceSymbols(symbolTable, embedded);
                char* serialized = SlimList_Serialize(embedded);
                SlimList_Iterator_Replace(it, serialized);
                SlimList_Destroy(embedded);
                SlimList_Release(serialized);
            }
        }
        SlimList_Iterator_Advance(&it);
    }
}

static std::string replaceString(SymbolTable* symbolTable, const char* s)
{
    if (!s) return "";
    return replaceStringFrom(symbolTable, s, 0);
}

static std::string replaceStringFrom(SymbolTable* symbolTable, const std::string& str, std::size_t from)
{
    std::size_t dollarPos = str.find('$', from);
    if (dollarPos == std::string::npos)
        return str;

    std::size_t nameEnd = dollarPos + 1;
    while (nameEnd < str.size() && std::isalnum(static_cast<unsigned char>(str[nameEnd])))
        nameEnd++;
    int length = static_cast<int>(nameEnd - dollarPos - 1);

    const char* symbolValue = SymbolTable_FindSymbol(symbolTable, str.c_str() + dollarPos + 1, length);
    if (symbolValue) {
        std::string newStr = str.substr(0, dollarPos) + symbolValue + str.substr(nameEnd);
        return replaceStringFrom(symbolTable, newStr, 0);
    }

    // Symbol not found: advance past '$' (or return if '$' is at end)
    if (dollarPos + 1 == str.size())
        return str;
    return replaceStringFrom(symbolTable, str, dollarPos + 1);
}

void* StatementExecutor_Instance(StatementExecutor* executor, char const* instanceName)
{
    InstanceInfo* node = GetInstanceNode(executor, instanceName);
    return node ? node->instance : nullptr;
}

void StatementExecutor_AddFixture(StatementExecutor* executor, Fixture fixture)
{
    fixture(executor);
}

void StatementExecutor_RegisterFixture(StatementExecutor* executor, char const* className, Constructor constructor, Destructor destructor)
{
    FixtureInfo* existing = findFixtureByName(executor, className);
    if (!existing) {
        executor->fixtures.push_back({className, constructor, destructor, {}});
    } else {
        existing->constructor = constructor;
        existing->destructor  = destructor;
    }
}

static FixtureInfo* findFixtureByName(StatementExecutor* executor, const char* className)
{
    for (auto& f : executor->fixtures)
        if (compareNamesIgnoreUnderScores(f.name.c_str(), className))
            return &f;
    return nullptr;
}

void StatementExecutor_RegisterMethod(StatementExecutor* executor, char const* className, char const* methodName, Method method)
{
    FixtureInfo* fixture = findFixtureByName(executor, className);
    if (!fixture) {
        executor->fixtures.push_back({className, Null_Create, Null_Destroy, {}});
        fixture = &executor->fixtures.back();
    }
    fixture->methods.push_back({methodName, method});
}

void StatementExecutor_SetSymbol(StatementExecutor* self, char const* symbol, char const* value)
{
    SymbolTable_SetSymbol(self->symbolTable, symbol, value);
}

void StatementExecutor_ConstructorError(StatementExecutor* executor, char const* message)
{
    executor->userMessage = message ? message : "";
}

const char* StatementExecutor_FixtureError(char const* message)
{
    static std::string buf;
    buf = std::string("__EXCEPTION__:message:<<") + message + ".>>";
    return buf.c_str();
}
