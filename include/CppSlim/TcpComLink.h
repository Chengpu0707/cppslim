#pragma once

typedef struct TcpComLink TcpComLink;

TcpComLink* TcpComLink_Create(int socket);
void TcpComLink_Destroy(TcpComLink*);
int  TcpComLink_send(void* voidSelf, const char* msg, int length);
int  TcpComLink_recv(void* voidSelf, char* buffer, int length);
