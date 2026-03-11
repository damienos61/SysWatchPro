#include "process.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static const char* SUSPICIOUS_NAMES[] = {
    "keylogger","rootkit","backdoor","trojan","miner","cryptominer",
    "stealer","rat.exe","spy","ransom","inject","hook","dump",
    "pwdump","mimikatz","netcat","nc.exe","ncat","psexec",NULL
};

/* suspicious paths: legitimate svchost must live in System32 */
static const char* SYSTEM_NAMES[] = {
    "svchost.exe","rundll32.exe","lsass.exe","csrss.exe",NULL
};

#define TRACK 1024
static DWORD          prevPids[TRACK];
static ULARGE_INTEGER prevSysK[TRACK], prevSysU[TRACK];
static ULARGE_INTEGER prevProcK[TRACK], prevProcU[TRACK];
static int            prevCount = 0;

/* first-seen registry */
static DWORD fsPids[TRACK];
static DWORD fsTick[TRACK];
static int   fsCount = 0;

static int find_prev(DWORD pid) {
    for (int i=0;i<prevCount;i++) if(prevPids[i]==pid) return i;
    return -1;
}
static DWORD get_first_seen(DWORD pid) {
    for (int i=0;i<fsCount;i++) if(fsPids[i]==pid) return fsTick[i];
    if (fsCount < TRACK) {
        fsPids[fsCount] = pid;
        fsTick[fsCount] = GetTickCount();
        fsCount++;
        return fsTick[fsCount-1];
    }
    return GetTickCount();
}

void process_list_init(ProcessList* pl) {
    memset(pl,0,sizeof(*pl));
    prevCount=0; fsCount=0;
}

int process_is_suspicious(const char* name) {
    char lo[MAX_NAME_LEN]; int i=0;
    while(name[i]&&i<MAX_NAME_LEN-1){lo[i]=(char)tolower((unsigned char)name[i]);i++;}
    lo[i]='\0';
    for(int j=0;SUSPICIOUS_NAMES[j];j++)
        if(strstr(lo,SUSPICIOUS_NAMES[j])) return 1;
    return 0;
}

static int check_path_anomaly(const char* name, const char* path) {
    if (!path || !path[0]) return 0;
    char loName[MAX_NAME_LEN]; int i=0;
    while(name[i]&&i<MAX_NAME_LEN-1){loName[i]=(char)tolower((unsigned char)name[i]);i++;}
    loName[i]='\0';
    char loPath[MAX_PATH_LEN]; i=0;
    while(path[i]&&i<MAX_PATH_LEN-1){loPath[i]=(char)tolower((unsigned char)path[i]);i++;}
    loPath[i]='\0';
    for(int j=0;SYSTEM_NAMES[j];j++) {
        char loSys[64]; int k=0;
        while(SYSTEM_NAMES[j][k]&&k<63){loSys[k]=(char)tolower((unsigned char)SYSTEM_NAMES[j][k]);k++;}
        loSys[k]='\0';
        if(strcmp(loName,loSys)==0) {
            if(!strstr(loPath,"system32") && !strstr(loPath,"syswow64"))
                return 1; /* system process outside system32 = suspicious */
        }
    }
    return 0;
}

static DWORD count_threads(DWORD pid) {
    HANDLE h=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    if(h==INVALID_HANDLE_VALUE) return 0;
    THREADENTRY32 te; te.dwSize=sizeof(te);
    DWORD cnt=0;
    if(Thread32First(h,&te)) do { if(te.th32OwnerProcessID==pid) cnt++; } while(Thread32Next(h,&te));
    CloseHandle(h);
    return cnt;
}

int process_kill(DWORD pid) {
    HANDLE h=OpenProcess(PROCESS_TERMINATE,FALSE,pid);
    if(!h) return 0;
    int ok=(TerminateProcess(h,1)!=0);
    CloseHandle(h); return ok;
}

int process_list_update(ProcessList* pl) {
    HANDLE hSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(hSnap==INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32 pe; pe.dwSize=sizeof(pe);
    pl->count=0;

    DWORD nPids[TRACK];
    ULARGE_INTEGER nSysK[TRACK],nSysU[TRACK],nProcK[TRACK],nProcU[TRACK];
    int nc=0;

    if(!Process32First(hSnap,&pe)){CloseHandle(hSnap);return 0;}
    do {
        if(pl->count>=MAX_PROCESSES) break;
        ProcessInfo* p=&pl->list[pl->count];
        p->pid=pe.th32ProcessID;
        strncpy(p->name,pe.szExeFile,MAX_NAME_LEN-1);
        p->name[MAX_NAME_LEN-1]='\0';
        p->suspicious=process_is_suspicious(p->name);
        p->threadCount=count_threads(p->pid);
        p->fullPath[0]='\0';
        p->memUsageKB=0; p->cpuUsage=0.0;
        p->firstSeenTick=get_first_seen(p->pid);
        p->isNew=((GetTickCount()-p->firstSeenTick)<60000)?1:0;

        HANDLE hProc=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,p->pid);
        if(hProc){
            DWORD sz=MAX_PATH_LEN;
            QueryFullProcessImageNameA(hProc,0,p->fullPath,&sz);

            PROCESS_MEMORY_COUNTERS pmc;
            if(GetProcessMemoryInfo(hProc,&pmc,sizeof(pmc)))
                p->memUsageKB=pmc.WorkingSetSize/1024;

            FILETIME ftI,ftK,ftU,ftCr,ftEx,ftPK,ftPU;
            GetSystemTimes(&ftI,&ftK,&ftU);
            GetProcessTimes(hProc,&ftCr,&ftEx,&ftPK,&ftPU);
            ULARGE_INTEGER sK,sU,pK,pU;
            sK.LowPart=ftK.dwLowDateTime; sK.HighPart=ftK.dwHighDateTime;
            sU.LowPart=ftU.dwLowDateTime; sU.HighPart=ftU.dwHighDateTime;
            pK.LowPart=ftPK.dwLowDateTime; pK.HighPart=ftPK.dwHighDateTime;
            pU.LowPart=ftPU.dwLowDateTime; pU.HighPart=ftPU.dwHighDateTime;
            if(nc<TRACK){
                int idx=find_prev(p->pid);
                if(idx>=0){
                    ULONGLONG sd=(sK.QuadPart-prevSysK[idx].QuadPart)+(sU.QuadPart-prevSysU[idx].QuadPart);
                    ULONGLONG pd=(pK.QuadPart-prevProcK[idx].QuadPart)+(pU.QuadPart-prevProcU[idx].QuadPart);
                    double cpu=sd>0?100.0*pd/sd:0.0;
                    p->cpuUsage=cpu>100.0?100.0:cpu;
                }
                nPids[nc]=p->pid; nSysK[nc]=sK; nSysU[nc]=sU; nProcK[nc]=pK; nProcU[nc]=pU; nc++;
            }
            /* path anomaly check */
            if(check_path_anomaly(p->name,p->fullPath)) p->suspicious=1;
            CloseHandle(hProc);
        }
        pl->count++;
    } while(Process32Next(hSnap,&pe));
    CloseHandle(hSnap);
    memcpy(prevPids,nPids,nc*sizeof(DWORD));
    memcpy(prevSysK,nSysK,nc*sizeof(ULARGE_INTEGER));
    memcpy(prevSysU,nSysU,nc*sizeof(ULARGE_INTEGER));
    memcpy(prevProcK,nProcK,nc*sizeof(ULARGE_INTEGER));
    memcpy(prevProcU,nProcU,nc*sizeof(ULARGE_INTEGER));
    prevCount=nc;
    return pl->count;
}
