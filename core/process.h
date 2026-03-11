#ifndef PROCESS_H
#define PROCESS_H

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

#define MAX_PROCESSES  1024
#define MAX_NAME_LEN   260
#define MAX_PATH_LEN   512
#define HISTORY_LEN    60

typedef struct {
    DWORD   pid;
    char    name[MAX_NAME_LEN];
    char    fullPath[MAX_PATH_LEN];
    double  cpuUsage;
    SIZE_T  memUsageKB;
    DWORD   threadCount;
    int     suspicious;
    int     anomalyScore;      /* 0-100 */
    DWORD   firstSeenTick;     /* GetTickCount at first appearance */
    int     isNew;             /* appeared in last 60s */
} ProcessInfo;

typedef struct {
    ProcessInfo list[MAX_PROCESSES];
    int         count;
} ProcessList;

/* WinAPI missing on old MinGW */
WINBASEAPI BOOL WINAPI GetSystemTimes(LPFILETIME,LPFILETIME,LPFILETIME);
WINBASEAPI BOOL WINAPI QueryFullProcessImageNameA(HANDLE,DWORD,LPSTR,PDWORD);

void process_list_init(ProcessList* pl);
int  process_list_update(ProcessList* pl);
int  process_is_suspicious(const char* name);
int  process_kill(DWORD pid);

#endif
