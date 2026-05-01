#include "SlimUtil.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

const char* CSlim_BuyBuf(char const* buffer, int length)
{
    if (buffer == NULL)
        return NULL;
    char* purchase = (char*)malloc(length + 1);
    strncpy(purchase, buffer, length);
    purchase[length] = '\0';
    return purchase;
}

const char* CSlim_BuyString(char const* string)
{
    if (string == NULL)
        return NULL;
    return CSlim_BuyBuf(string, (int)strlen(string));
}

const char* CSlim_CreateEmptyString()
{
    char* result = (char*)malloc(sizeof(char));
    assert(result != NULL);
    result[0] = '\0';
    return result;
}

void CSlim_DestroyString(const char* string)
{
    free((void*)string);
}

void CSlim_ConcatenateString(const char** toAppendTo, const char* toAppend)
{
    size_t requiredLength = strlen(*toAppendTo) + strlen(toAppend) + 1;
    char* temp = (char*)realloc((char*)*toAppendTo, requiredLength);
    assert(temp != NULL);
    strcat(temp, toAppend);
    *toAppendTo = temp;
}

int CSlim_StringStartsWith(const char* string, const char* prefix)
{
    return strncmp(string, prefix, strlen(prefix)) == 0;
}

int CSlim_MapToIntFrom(MapStringInt* map, const char* name)
{
    MapStringInt* p = map;
    while (p->string != NULL && 0 != strcmp(name, p->string))
        p++;
    return p->n;
}

const char* CSlim_MapToStringFrom(MapStringInt* map, int n)
{
    MapStringInt* p = map;
    while (p->string != NULL && n != p->n)
        p++;
    return p->string;
}

int CSlim_IsCharacter(unsigned char const* byte)
{
    if ((*byte < (unsigned char)0x80) || (*byte > (unsigned char)0xBF))
        return 1;
    return 0;
}
