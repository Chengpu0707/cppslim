#pragma once
#include "StatementExecutor.h"
#include "SlimList.h"

struct TestSlim;

void* TestSlim_Create(StatementExecutor* executor, SlimList* args);
void  TestSlim_Destroy(void*);
void  TestSlim_Register(StatementExecutor*);

int   TestSlim_noArgsCalled(TestSlim* self);
