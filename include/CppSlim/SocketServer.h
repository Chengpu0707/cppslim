#pragma once

typedef struct SocketServer SocketServer;

SocketServer* SocketServer_Create();
void SocketServer_Destroy(SocketServer*);
int  SocketServer_Run(SocketServer* self, char* port);
void SocketServer_register_handler(SocketServer* self, int (*handlerFunction)(int));
