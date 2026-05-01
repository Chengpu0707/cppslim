#include "ListExecutor.h"
#include <string>
#include <cstring>
#include <cstdlib>

struct ListExecutor {
    StatementExecutor* executor;
};

ListExecutor* ListExecutor_Create(StatementExecutor* executor)
{
    return new ListExecutor{executor};
}

void ListExecutor_Destroy(ListExecutor* self)
{
    delete self;
}

static void AddResult(SlimList* list, const char* id, const std::string& result)
{
    SlimList* pair = SlimList_Create();
    SlimList_AddString(pair, id);
    SlimList_AddString(pair, result.c_str());
    SlimList_AddList(list, pair);
    SlimList_Destroy(pair);
}

static std::string InvalidCommand(SlimList* instruction)
{
    const char* id      = SlimList_GetStringAt(instruction, 0);
    const char* command = SlimList_GetStringAt(instruction, 1);
    return std::string("__EXCEPTION__:message:<<INVALID_STATEMENT: [\"")
        + (id ? id : "") + "\", \"" + (command ? command : "") + "\"].>>";
}

static std::string MalformedInstruction(SlimList* instruction)
{
    const char* s = SlimList_ToString(instruction);
    std::string result = std::string("__EXCEPTION__:message:<<MALFORMED_INSTRUCTION ") + s + ".>>";
    std::free(const_cast<char*>(s));
    return result;
}

static std::string Import()
{
    return "OK";
}

static std::string nullsafe(const char* s) { return s ? s : "null"; }

static std::string Make(ListExecutor* self, SlimList* instruction)
{
    const char* instanceName = SlimList_GetStringAt(instruction, 2);
    const char* className    = SlimList_GetStringAt(instruction, 3);
    SlimList*   args         = SlimList_GetTailAt(instruction, 4);
    std::string result = nullsafe(StatementExecutor_Make(self->executor, instanceName, className, args));
    SlimList_Destroy(args);
    return result;
}

static std::string Call(ListExecutor* self, SlimList* instruction)
{
    if (SlimList_GetLength(instruction) < 4)
        return MalformedInstruction(instruction);
    const char* instanceName = SlimList_GetStringAt(instruction, 2);
    const char* methodName   = SlimList_GetStringAt(instruction, 3);
    SlimList*   args         = SlimList_GetTailAt(instruction, 4);
    std::string result = nullsafe(StatementExecutor_Call(self->executor, instanceName, methodName, args));
    SlimList_Destroy(args);
    return result;
}

static std::string CallAndAssign(ListExecutor* self, SlimList* instruction)
{
    if (SlimList_GetLength(instruction) < 5)
        return MalformedInstruction(instruction);
    const char* symbolName   = SlimList_GetStringAt(instruction, 2);
    const char* instanceName = SlimList_GetStringAt(instruction, 3);
    const char* methodName   = SlimList_GetStringAt(instruction, 4);
    SlimList*   args         = SlimList_GetTailAt(instruction, 5);
    std::string result = nullsafe(StatementExecutor_Call(self->executor, instanceName, methodName, args));
    StatementExecutor_SetSymbol(self->executor, symbolName, result.c_str());
    SlimList_Destroy(args);
    return result;
}

static std::string Dispatch(ListExecutor* self, SlimList* instruction)
{
    const char* command = SlimList_GetStringAt(instruction, 1);
    if (!command)                              return InvalidCommand(instruction);
    if (strcmp(command, "import") == 0)        return Import();
    if (strcmp(command, "make") == 0)          return Make(self, instruction);
    if (strcmp(command, "call") == 0)          return Call(self, instruction);
    if (strcmp(command, "callAndAssign") == 0) return CallAndAssign(self, instruction);
    return InvalidCommand(instruction);
}

SlimList* ListExecutor_Execute(ListExecutor* self, SlimList* instructions)
{
    SlimList* results = SlimList_Create();
    SlimListIterator* it = SlimList_CreateIterator(instructions);
    while (SlimList_Iterator_HasItem(it)) {
        SlimList*   instruction = SlimList_Iterator_GetList(it);
        const char* id          = SlimList_GetStringAt(instruction, 0);
        std::string result      = Dispatch(self, instruction);
        AddResult(results, id, result);
        SlimList_Iterator_Advance(&it);
    }
    return results;
}
