#include "CppFixtures.h"

// Called by Slim_Create() in Slim.cpp.
void AddFixtures(StatementExecutor* executor) {
    SLIM_INCLUDE_FIXTURE(Count);
    SLIM_INCLUDE_FIXTURE(Division);
    SLIM_INCLUDE_FIXTURE(EmployeePayRecords);
}
