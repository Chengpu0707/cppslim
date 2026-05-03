// Decision-table fixture: mirrors cslim's DecisionTableExample.c
// Setters throw on invalid input; lifecycle methods (execute/reset/table) are optional.
#include <stdexcept>
#include <vector>
#include <string>
#include "CppFixtures.h"

class Division : public SlimFixture<Division> {
    double numerator_   = 0.0;
    double denominator_ = 1.0;

public:
    explicit Division(const std::vector<std::string>&) {}

    void   setNumerator(double d)   { numerator_ = d; }
    void        setDenominator(double d) { denominator_ = d; }
    std::string quotient() const {
        try {
            if (denominator_ == 0.0)
                throw std::runtime_error("You shouldn't divide by zero now should ya?");
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%g", numerator_ / denominator_);
            return buf;
        } catch (const std::exception& e) {
            return e.what();
        }
    }

    // Optional decision-table lifecycle hooks
    void execute() {}
    void reset()   { numerator_ = 0.0; denominator_ = 1.0; }
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
