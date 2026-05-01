// Query-table fixture: mirrors cslim's QueryTableExample.c
// query() builds a nested SlimList, serializes it to a string, and returns it.
// SlimConvert<std::string> passes the serialized wire-format string straight through.
#include <string>
#include <vector>
#include "SlimList.h"
#include "SlimListSerializer.h"
#include "CppFixtures.h"

class EmployeePayRecords : public SlimFixture<EmployeePayRecords> {
public:
    explicit EmployeePayRecords(const std::vector<std::string>&) {}

    std::string query() {
        // Build: [[["id","1"],["pay","1000"]]]
        SlimList* id = SlimList_Create();
        SlimList_AddString(id, "id");
        SlimList_AddString(id, "1");

        SlimList* pay = SlimList_Create();
        SlimList_AddString(pay, "pay");
        SlimList_AddString(pay, "1000");

        SlimList* record = SlimList_Create();
        SlimList_AddList(record, id);
        SlimList_AddList(record, pay);

        SlimList* records = SlimList_Create();
        SlimList_AddList(records, record);

        char* raw = SlimList_Serialize(records);
        std::string result(raw);
        SlimList_Release(raw);

        SlimList_Destroy(records);
        SlimList_Destroy(record);
        SlimList_Destroy(pay);
        SlimList_Destroy(id);

        return result;
    }
};

SLIM_FIXTURE(EmployeePayRecords)
    SLIM_METHOD(query)
SLIM_END
