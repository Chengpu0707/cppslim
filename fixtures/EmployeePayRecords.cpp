// Query-table fixture: mirrors cslim's QueryTableExample.c
#include <string>
#include <vector>
#include "SlimList.h"
#include "CppFixtures.h"

class EmployeePayRecords : public SlimFixture<EmployeePayRecords> {
public:
    explicit EmployeePayRecords(const std::vector<std::string>&) {}

    std::string query() {
        SlimList* id = new SlimList();
        id->addString("id");
        id->addString("1");

        SlimList* pay = new SlimList();
        pay->addString("pay");
        pay->addString("1000");

        SlimList* record = new SlimList();
        record->addList(id);
        record->addList(pay);

        SlimList* records = new SlimList();
        records->addList(record);

        char* raw = records->serialize();
        std::string result(raw);
        SlimList::release(raw);

        delete records;
        delete record;
        delete pay;
        delete id;

        return result;
    }
};

SLIM_FIXTURE(EmployeePayRecords)
    SLIM_METHOD(query)
SLIM_END
