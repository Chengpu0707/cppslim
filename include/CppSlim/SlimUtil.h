#pragma once

const char* CSlim_BuyBuf(char const* buffer, int length);
const char* CSlim_BuyString(char const* str);
const char* CSlim_CreateEmptyString();
void CSlim_ConcatenateString(const char** toAppendTo, const char* toAppend);
int CSlim_StringStartsWith(const char* string, const char* prefix);
void CSlim_DestroyString(const char* string);

struct MapStringInt {
    const char* string;
    int n;
};

int CSlim_MapToIntFrom(MapStringInt* map, const char* name);
const char* CSlim_MapToStringFrom(MapStringInt* map, int n);
int CSlim_IsCharacter(unsigned char const* byte);
