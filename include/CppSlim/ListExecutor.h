#pragma once
#include "SlimList.h"
#include "StatementExecutor.h"
#include <memory>

class ListExecutor {
public:
    explicit ListExecutor(StatementExecutor* executor);
    std::unique_ptr<SlimList> execute(SlimList* instructions);

private:
    StatementExecutor* executor_;
};
