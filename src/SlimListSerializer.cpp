#include "SlimList.h"
#include "SlimUtil.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const char* stringForNullNode = "null";
enum { LIST_OVERHEAD = 9, ELEMENT_OVERHEAD = 8 };

static const char* withNullAsString(SlimListIterator* it)
{
    const char* s = it->getString();
    return s ? s : stringForNullNode;
}

static long fieldLength(unsigned char* s)
{
    long len = 0;
    for (unsigned char* p = s; *p; ++p)
        if (CSlim_IsCharacter(p) == 1)
            ++len;
    return len;
}

int SlimList::serializedLength() const
{
    int len = LIST_OVERHEAD;
    for (auto* it = createIterator(); it != nullptr; it = it->advance())
        len += (int)strlen(withNullAsString(it)) + ELEMENT_OVERHEAD;
    return len;
}

char* SlimList::serialize() const
{
    char* buf = (char*)malloc(serializedLength() + 1);
    char* wp  = buf;
    wp += sprintf(wp, "[%06d:", getLength());
    for (auto* it = createIterator(); it != nullptr; it = it->advance()) {
        unsigned char* s = (unsigned char*)withNullAsString(it);
        wp += sprintf(wp, "%06ld:%s:", fieldLength(s), s);
    }
    strcpy(wp, "]");
    return buf;
}

void SlimList::release(char* serialized)
{
    free(serialized);
}
