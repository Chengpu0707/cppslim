// Query-table fixture: mirrors cslim's QueryTableExample.c
#include <string>
#include <vector>
#include "SlimList.h"
#include "CppFixtures.h"

class EmployeePayRecords : public SlimFixture<EmployeePayRecords> {
public:
    explicit EmployeePayRecords(const std::vector<std::string>&) {}

    std::string query() {
        SlimList id, pay, record, records;
        id.addString("id");      id.addString("1");
        pay.addString("pay");    pay.addString("1000");
        record.addList(&id);     record.addList(&pay);
        records.addList(&record);
        return records.serialize();
    }
};

SLIM_FIXTURE(EmployeePayRecords)
    SLIM_METHOD(query)
SLIM_END
