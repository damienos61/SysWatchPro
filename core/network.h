#ifndef NETWORK_H
#define NETWORK_H

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#define MAX_CONNECTIONS 512

typedef struct {
    DWORD  pid;
    char   procName[64];
    char   localAddr[46];
    USHORT localPort;
    char   remoteAddr[46];
    USHORT remotePort;
    char   state[24];
    int    suspicious;   /* external IP on unusual port */
} NetConnection;

typedef struct {
    NetConnection list[MAX_CONNECTIONS];
    int           count;
    int           totalTCP;
    int           totalUDP;
    int           suspiciousCount;
} NetInfo;

int  network_info_init(NetInfo* ni);
int  network_info_update(NetInfo* ni, const char procNames[][64], const DWORD* pids, int procCount);
void network_info_print(const NetInfo* ni, int topN);

#endif
