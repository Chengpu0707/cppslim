#pragma once
#include "SlimList.h"

char* SlimList_Serialize(SlimList*);
void  SlimList_Release(char* serializedResults);
int   SlimList_SerializedLength(SlimList*);
