// Script-table fixture: mirrors cslim's ScriptTableExample.c
#include <vector>
#include <string>
#include "CppFixtures.h"

class Count : public SlimFixture<Count> {
    int count_ = 0;

public:
    explicit Count(const std::vector<std::string>&) {}

    void count()        { ++count_; }
    int  counter() const { return count_; }
};

SLIM_FIXTURE(Count)
    SLIM_METHOD(count)
    SLIM_METHOD(counter)
SLIM_END
