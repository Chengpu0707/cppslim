#include "StatementExecutor.h"
#include "SymbolTable.h"
#include <cstring>
#include <cctype>
#include <cassert>

static std::string trunc32(const char* s)
{
    if (!s) return "";
    std::size_t len = std::strlen(s);
    return std::string(s, len > 32 ? 32 : len);
}

static bool compareNamesIgnoreUnderScores(const char* a, const char* b)
{
    while (*a && *b) {
        if (*a == *b)       { ++a; ++b; }
        else if (*a == '_') { ++a; }
        else if (*b == '_') { ++b; }
        else return false;
    }
    return *a == *b;
}

static bool isLibraryInstanceName(const char* name)
{
    return std::strncmp(name, "library", 7) == 0;
}

static void* Null_Create(StatementExecutor*, SlimList*) { return nullptr; }
static void  Null_Destroy(void*) {}

// ---------------------------------------------------------------------------

StatementExecutor::StatementExecutor()
{
    symbolTable_ = new SymbolTable();
}

StatementExecutor::~StatementExecutor()
{
    for (auto& inst : libraryInstances_)
        inst.fixture->destructor(inst.instance);
    for (auto& inst : instances_)
        inst.fixture->destructor(inst.instance);
    delete symbolTable_;
}

void StatementExecutor::addFixture(Fixture fixture)
{
    fixture(this);
}

void StatementExecutor::registerFixture(const char* className, Constructor ctor, Destructor dtor)
{
    FixtureInfo* existing = findFixtureByName(className);
    if (!existing) {
        fixtures_.push_back({className, ctor, dtor, {}});
    } else {
        existing->constructor = ctor;
        existing->destructor  = dtor;
    }
}

void StatementExecutor::registerMethod(const char* className, const char* methodName, Method method)
{
    FixtureInfo* fixture = findFixtureByName(className);
    if (!fixture) {
        fixtures_.push_back({className, Null_Create, Null_Destroy, {}});
        fixture = &fixtures_.back();
    }
    fixture->methods.push_back({methodName, method});
}

const char* StatementExecutor::make(const char* instanceName, const char* className, SlimList* args)
{
    FixtureInfo* fixture = findFixture(className);
    if (!fixture) {
        message_ = std::string("__EXCEPTION__:message:<<NO_CLASS ") + trunc32(className) + ".>>";
        return message_.c_str();
    }
    replaceSymbols(args);
    userMessage_.clear();
    void* inst = fixture->constructor(this, args);

    InstanceInfo info{instanceName, inst, fixture};
    if (isLibraryInstanceName(instanceName))
        libraryInstances_.push_front(std::move(info));
    else
        instances_.push_front(std::move(info));

    if (inst != nullptr)
        return "OK";
    message_ = std::string("__EXCEPTION__:message:<<COULD_NOT_INVOKE_CONSTRUCTOR ")
        + trunc32(className) + " " + trunc32(userMessage_.c_str()) + ".>>";
    return message_.c_str();
}

const char* StatementExecutor::call(const char* instanceName, const char* methodName, SlimList* args)
{
    InstanceInfo* inst = getInstanceNode(instanceName);
    if (inst) {
        for (auto& m : inst->fixture->methods) {
            if (compareNamesIgnoreUnderScores(methodName, m.name.c_str()))
                return invokeMethod(&m, inst, args);
        }
        for (auto& lib : libraryInstances_) {
            for (auto& m : lib.fixture->methods) {
                if (compareNamesIgnoreUnderScores(methodName, m.name.c_str()))
                    return invokeMethod(&m, &lib, args);
            }
        }
        message_ = std::string("__EXCEPTION__:message:<<NO_METHOD_IN_CLASS ")
            + trunc32(methodName) + "[" + std::to_string(args->getLength()) + "] "
            + trunc32(inst->fixture->name.c_str()) + ".>>";
        return message_.c_str();
    }
    message_ = std::string("__EXCEPTION__:message:<<NO_INSTANCE ") + trunc32(instanceName) + ".>>";
    return message_.c_str();
}

void* StatementExecutor::instance(const char* instanceName)
{
    InstanceInfo* node = getInstanceNode(instanceName);
    return node ? node->instance : nullptr;
}

void StatementExecutor::setSymbol(const char* symbol, const char* value)
{
    symbolTable_->setSymbol(symbol, value);
}

void StatementExecutor::constructorError(const char* message)
{
    userMessage_ = message ? message : "";
}

const char* StatementExecutor::fixtureError(const char* message)
{
    static std::string buf;
    buf = std::string("__EXCEPTION__:message:<<") + message + ".>>";
    return buf.c_str();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

StatementExecutor::FixtureInfo* StatementExecutor::findFixtureByName(const char* className)
{
    for (auto& f : fixtures_)
        if (compareNamesIgnoreUnderScores(f.name.c_str(), className))
            return &f;
    return nullptr;
}

StatementExecutor::FixtureInfo* StatementExecutor::findFixture(const char* classNameWithSymbols)
{
    std::string className = replaceString(classNameWithSymbols);
    return findFixtureByName(className.c_str());
}

StatementExecutor::InstanceInfo* StatementExecutor::getInstanceNode(const char* instanceName)
{
    for (auto& inst : instances_)
        if (compareNamesIgnoreUnderScores(inst.name.c_str(), instanceName))
            return &inst;
    return nullptr;
}

const char* StatementExecutor::invokeMethod(MethodInfo* method, InstanceInfo* inst, SlimList* args)
{
    replaceSymbols(args);
    return method->method(inst->instance, args);
}

void StatementExecutor::replaceSymbols(SlimList* list)
{
    for (auto* it = list->createIterator(); it != nullptr; it = it->advance()) {
        const char* s = it->getString();
        if (s != nullptr) {
            SlimList* embedded = SlimList::deserialize(s);
            if (!embedded) {
                it->replace(replaceString(s).c_str());
            } else {
                replaceSymbols(embedded);
                char* serial = embedded->serialize();
                it->replace(serial);
                delete embedded;
                SlimList::release(serial);
            }
        }
    }
}

std::string StatementExecutor::replaceString(const char* s)
{
    if (!s) return "";
    return replaceStringFrom(s, 0);
}

std::string StatementExecutor::replaceStringFrom(const std::string& str, std::size_t from)
{
    std::size_t dollarPos = str.find('$', from);
    if (dollarPos == std::string::npos)
        return str;

    std::size_t nameEnd = dollarPos + 1;
    while (nameEnd < str.size() && std::isalnum(static_cast<unsigned char>(str[nameEnd])))
        ++nameEnd;
    int length = static_cast<int>(nameEnd - dollarPos - 1);

    const char* value = symbolTable_->findSymbol(str.c_str() + dollarPos + 1, length);
    if (value) {
        std::string newStr = str.substr(0, dollarPos) + value + str.substr(nameEnd);
        return replaceStringFrom(newStr, 0);
    }
    if (dollarPos + 1 == str.size())
        return str;
    return replaceStringFrom(str, dollarPos + 1);
}
