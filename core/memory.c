#include "memory.h"
#include "../gui/widgets.h"
#include <stdio.h>
#include <string.h>

int memory_info_init(MemoryInfo* mi){ memset(mi,0,sizeof(*mi)); return memory_info_update(mi); }

int memory_info_update(MemoryInfo* mi){
    MEMORYSTATUSEX ms; ms.dwLength=sizeof(ms);
    if(!GlobalMemoryStatusEx(&ms)) return 0;
    mi->totalPhysKB=ms.ullTotalPhys/1024;
    mi->freePhysKB=ms.ullAvailPhys/1024;
    mi->usedPhysKB=mi->totalPhysKB-mi->freePhysKB;
    mi->usagePercent=mi->totalPhysKB>0?100.0*mi->usedPhysKB/mi->totalPhysKB:0.0;
    mi->totalVirtKB=ms.ullTotalVirtual/1024;
    mi->usedVirtKB=(ms.ullTotalVirtual-ms.ullAvailVirtual)/1024;
    mi->history[mi->histIdx]=mi->usagePercent;
    mi->histIdx=(mi->histIdx+1)%HISTORY_LEN;
    if(mi->histFilled<HISTORY_LEN) mi->histFilled++;
    return 1;
}

void memory_draw_bar(double percent, int width){ widget_draw_bar(percent,width); }

void memory_draw_graph(const MemoryInfo* mi, int width){
    static const char* bl[]={"_",".",":","-","=","x","X","#","@"};
    int count=mi->histFilled<width?mi->histFilled:width;
    int start=mi->histIdx-count; if(start<0) start+=HISTORY_LEN;
    printf("  RAM 60s [");
    for(int i=0;i<width;i++){
        if(i>=count){widget_set_color(8);putchar(' ');widget_reset_color();continue;}
        int idx=(start+i)%HISTORY_LEN;
        double v=mi->history[idx];
        int lv=(int)(v/100.0*8); if(lv>8)lv=8;
        if(v<60.0)widget_set_color(10);
        else if(v<85.0)widget_set_color(14);
        else widget_set_color(12);
        printf("%s",bl[lv]);
        widget_reset_color();
    }
    printf("]\n");
}

static void print_kb(DWORDLONG kb){
    if(kb>=1024ULL*1024) printf("%.2f GB",(double)kb/(1024.0*1024));
    else if(kb>=1024)    printf("%.1f MB",(double)kb/1024.0);
    else                 printf("%llu KB",(unsigned long long)kb);
}

void memory_info_print(const MemoryInfo* mi){
    printf("  Totale  : "); print_kb(mi->totalPhysKB); printf("\n");
    printf("  Utilisee: "); print_kb(mi->usedPhysKB);  printf("\n");
    printf("  Libre   : "); print_kb(mi->freePhysKB);  printf("\n");
    printf("  Charge  : "); memory_draw_bar(mi->usagePercent,45); printf("\n");
    memory_draw_graph(mi,60);
    printf("  Virtuel : "); print_kb(mi->usedVirtKB); printf(" / "); print_kb(mi->totalVirtKB); printf("\n");
}
