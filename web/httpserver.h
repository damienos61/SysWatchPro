#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <windows.h>
#include "../core/cpu.h"
#include "../core/memory.h"
#include "../core/process.h"
#include "../core/network.h"
#include "../core/anomaly.h"

#define HTTP_PORT 8080

typedef struct {
    CpuInfo*      cpu;
    MemoryInfo*   mem;
    ProcessList*  procs;
    NetInfo*      net;
    AnomalyReport* anomaly;
    volatile int  running;
} HttpServerState;

int  http_server_start(HttpServerState* state);
void http_server_stop(HttpServerState* state);

#endif
