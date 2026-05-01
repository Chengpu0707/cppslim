#include "SlimList.h"
#include "SlimUtil.h"
#include "SlimListSerializer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const char* stringForNullNode = "null";
enum { LIST_OVERHEAD = 9, ELEMENT_OVERHEAD = 8 };

static const char* GetStringWithNullAsANormalString(SlimListIterator* iterator)
{
    const char* s = SlimList_Iterator_GetString(iterator);
    return s ? s : stringForNullNode;
}

static long FieldLength(unsigned char* nodeString)
{
    unsigned char* p;
    long len = 0;
    for (p = nodeString; *p; p++)
        if (CSlim_IsCharacter(p) == 1)
            len++;
    return len;
}

int SlimList_SerializedLength(SlimList* self)
{
    int length = LIST_OVERHEAD;
    SlimListIterator* it = SlimList_CreateIterator(self);
    while (SlimList_Iterator_HasItem(it)) {
        length += (int)strlen(GetStringWithNullAsANormalString(it)) + ELEMENT_OVERHEAD;
        SlimList_Iterator_Advance(&it);
    }
    return length;
}

char* SlimList_Serialize(SlimList* self)
{
    char* buf = (char*)malloc(SlimList_SerializedLength(self) + 1);
    char* wp = buf;
    wp += sprintf(wp, "[%06d:", SlimList_GetLength(self));
    SlimListIterator* it = SlimList_CreateIterator(self);
    while (SlimList_Iterator_HasItem(it)) {
        unsigned char* s = (unsigned char*)GetStringWithNullAsANormalString(it);
        wp += sprintf(wp, "%06ld:%s:", FieldLength(s), s);
        SlimList_Iterator_Advance(&it);
    }
    strcpy(wp, "]");
    return buf;
}

void SlimList_Release(char* serializedResults)
{
    if (serializedResults)
        free(serializedResults);
}
