#include "SlimList.h"
#include "SlimUtil.h"
#include <stdlib.h>
#include <string.h>

#define SKIP(a) \
    if (*cur != (a)) { delete list; return nullptr; } \
    cur++;

static int readLength(const char** p)
{
    int n = atoi(*p);
    *p += 6;
    return n;
}

static int byteLength(int charLen, const char* cur)
{
    const unsigned char* p = (const unsigned char*)cur;
    int chars = 0, bytes = 0;
    for (; chars <= charLen; ++p) {
        ++bytes;
        if (CSlim_IsCharacter(p) == 1)
            ++chars;
    }
    if (chars > charLen)
        --bytes;
    return bytes;
}

SlimList* SlimList::deserialize(const char* s)
{
    if (!s || strlen(s) == 0)
        return nullptr;

    const char* cur  = s;
    SlimList*   list = new SlimList();

    SKIP('[')
    int listLen = readLength(&cur);
    SKIP(':')

    while (listLen--) {
        int charLen = readLength(&cur);
        SKIP(':')
        int bLen = byteLength(charLen, cur);
        list->addBuffer(cur, bLen);
        cur += bLen;
        SKIP(':')
    }

    SKIP(']')
    return list;
}
