// Decision-table fixture: mirrors cslim's DecisionTableExample.c
// Setters throw on invalid input; lifecycle methods (execute/reset/table) are optional.
#include <vector>
#include <string>
#include "CppFixtures.h"

class Division : public SlimFixture<Division> {
    double numerator_   = 0.0;
    double denominator_ = 1.0;

public:
    explicit Division(const std::vector<std::string>&) {}

    void   setNumerator(double d)   { numerator_ = d; }
    void   setDenominator(double d) { denominator_ = d; }
    double quotient() const         { return numerator_ / denominator_; }

    // Optional decision-table lifecycle hooks
    void execute() {}
    void reset()   { numerator_ = 0.0; denominator_ = 0.0; }
    void table()   {}
};

SLIM_FIXTURE(Division)
    SLIM_METHOD(setNumerator)
    SLIM_METHOD(setDenominator)
    SLIM_METHOD(quotient)
    SLIM_METHOD(execute)
    SLIM_METHOD(reset)
    SLIM_METHOD(table)
SLIM_END
