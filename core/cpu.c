#include "cpu.h"
#include "../gui/widgets.h"
#include <stdio.h>
#include <string.h>

WINBASEAPI BOOL WINAPI GetSystemTimes(LPFILETIME,LPFILETIME,LPFILETIME);

static ULARGE_INTEGER prevIdle,prevKernel,prevUser;
static int initialized=0;

void cpu_info_init(CpuInfo* ci){
    SYSTEM_INFO si; GetSystemInfo(&si);
    ci->coreCount=(int)si.dwNumberOfProcessors;
    ci->totalUsage=0.0; ci->histIdx=0; ci->histFilled=0;
    memset(ci->history,0,sizeof(ci->history));
    FILETIME ftI,ftK,ftU;
    if(GetSystemTimes(&ftI,&ftK,&ftU)){
        prevIdle.LowPart=ftI.dwLowDateTime; prevIdle.HighPart=ftI.dwHighDateTime;
        prevKernel.LowPart=ftK.dwLowDateTime; prevKernel.HighPart=ftK.dwHighDateTime;
        prevUser.LowPart=ftU.dwLowDateTime; prevUser.HighPart=ftU.dwHighDateTime;
        initialized=1;
    }
}

int cpu_info_update(CpuInfo* ci){
    FILETIME ftI,ftK,ftU;
    if(!GetSystemTimes(&ftI,&ftK,&ftU)) return 0;
    ULARGE_INTEGER idle,kernel,user;
    idle.LowPart=ftI.dwLowDateTime; idle.HighPart=ftI.dwHighDateTime;
    kernel.LowPart=ftK.dwLowDateTime; kernel.HighPart=ftK.dwHighDateTime;
    user.LowPart=ftU.dwLowDateTime; user.HighPart=ftU.dwHighDateTime;
    if(!initialized){prevIdle=idle;prevKernel=kernel;prevUser=user;initialized=1;ci->totalUsage=0.0;}
    else {
        ULONGLONG dI=idle.QuadPart-prevIdle.QuadPart;
        ULONGLONG dK=kernel.QuadPart-prevKernel.QuadPart;
        ULONGLONG dU=user.QuadPart-prevUser.QuadPart;
        ULONGLONG total=dK+dU;
        ci->totalUsage=total>0?100.0*(total-dI)/total:0.0;
        prevIdle=idle; prevKernel=kernel; prevUser=user;
    }
    ci->history[ci->histIdx]=ci->totalUsage;
    ci->histIdx=(ci->histIdx+1)%HISTORY_LEN;
    if(ci->histFilled<HISTORY_LEN) ci->histFilled++;
    return 1;
}

void cpu_draw_bar(double percent, int width){ widget_draw_bar(percent,width); }

void cpu_draw_graph(const CpuInfo* ci, int width){
    static const char* bl[]={"_",".",":","-","=","x","X","#","@"};
    int count=ci->histFilled<width?ci->histFilled:width;
    int start=ci->histIdx-count; if(start<0) start+=HISTORY_LEN;
    printf("  CPU 60s [");
    for(int i=0;i<width;i++){
        if(i>=count){widget_set_color(8);putchar(' ');widget_reset_color();continue;}
        int idx=(start+i)%HISTORY_LEN;
        double v=ci->history[idx];
        int lv=(int)(v/100.0*8); if(lv>8)lv=8;
        if(v<50.0)widget_set_color(10);
        else if(v<80.0)widget_set_color(14);
        else widget_set_color(12);
        printf("%s",bl[lv]);
        widget_reset_color();
    }
    printf("]\n");
}

void cpu_info_print(const CpuInfo* ci){
    printf("  Coeurs : %d\n",ci->coreCount);
    printf("  CPU    : "); cpu_draw_bar(ci->totalUsage,45); printf("\n");
    cpu_draw_graph(ci,60);
}
