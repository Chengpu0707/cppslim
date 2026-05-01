#include "ListExecutor.h"
#include <string>
#include <cstring>

ListExecutor::ListExecutor(StatementExecutor* executor)
    : executor_(executor)
{}

static void addResult(SlimList* list, const char* id, const std::string& result)
{
    SlimList pair;
    pair.addString(id);
    pair.addString(result.c_str());
    list->addList(&pair);
}

static std::string invalidCommand(SlimList* instruction)
{
    const char* id  = instruction->getStringAt(0);
    const char* cmd = instruction->getStringAt(1);
    return std::string("__EXCEPTION__:message:<<INVALID_STATEMENT: [\"")
        + (id ? id : "") + "\", \"" + (cmd ? cmd : "") + "\"].>>";
}

static std::string malformedInstruction(SlimList* instruction)
{
    return std::string("__EXCEPTION__:message:<<MALFORMED_INSTRUCTION ")
        + instruction->toString() + ".>>";
}

static std::string nullsafe(const char* s) { return s ? s : "null"; }

static std::string doImport() { return "OK"; }

static std::string doMake(StatementExecutor* executor, SlimList* instruction)
{
    const char* instanceName = instruction->getStringAt(2);
    const char* className    = instruction->getStringAt(3);
    SlimList args = instruction->getTailAt(4);
    return nullsafe(executor->make(instanceName, className, &args));
}

static std::string doCall(StatementExecutor* executor, SlimList* instruction)
{
    if (instruction->getLength() < 4)
        return malformedInstruction(instruction);
    const char* instanceName = instruction->getStringAt(2);
    const char* methodName   = instruction->getStringAt(3);
    SlimList args = instruction->getTailAt(4);
    return nullsafe(executor->call(instanceName, methodName, &args));
}

static std::string doCallAndAssign(StatementExecutor* executor, SlimList* instruction)
{
    if (instruction->getLength() < 5)
        return malformedInstruction(instruction);
    const char* symbolName   = instruction->getStringAt(2);
    const char* instanceName = instruction->getStringAt(3);
    const char* methodName   = instruction->getStringAt(4);
    SlimList args = instruction->getTailAt(5);
    std::string result = nullsafe(executor->call(instanceName, methodName, &args));
    executor->setSymbol(symbolName, result.c_str());
    return result;
}

static std::string dispatch(StatementExecutor* executor, SlimList* instruction)
{
    const char* cmd = instruction->getStringAt(1);
    if (!cmd)                              return invalidCommand(instruction);
    if (strcmp(cmd, "import") == 0)        return doImport();
    if (strcmp(cmd, "make") == 0)          return doMake(executor, instruction);
    if (strcmp(cmd, "call") == 0)          return doCall(executor, instruction);
    if (strcmp(cmd, "callAndAssign") == 0) return doCallAndAssign(executor, instruction);
    return invalidCommand(instruction);
}

SlimList ListExecutor::execute(SlimList* instructions)
{
    SlimList results;
    for (auto* it = instructions->createIterator(); it != nullptr; it = it->advance()) {
        SlimList*   instruction = it->getList();
        const char* id          = instruction->getStringAt(0);
        std::string result      = dispatch(executor_, instruction);
        addResult(&results, id, result);
    }
    return results;
}
