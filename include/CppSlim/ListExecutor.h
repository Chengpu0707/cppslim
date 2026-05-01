#pragma once
#include "SlimList.h"
#include "StatementExecutor.h"

class ListExecutor {
public:
    explicit ListExecutor(StatementExecutor* executor);
    SlimList execute(SlimList* instructions);

private:
    StatementExecutor* executor_;
};
