#ifndef MEMORY_H
#define MEMORY_H
#include <windows.h>
#include "cpu.h"

typedef struct {
    DWORDLONG totalPhysKB, usedPhysKB, freePhysKB;
    double    usagePercent;
    DWORDLONG totalVirtKB, usedVirtKB;
    double history[HISTORY_LEN];
    int    histIdx, histFilled;
} MemoryInfo;

int  memory_info_init(MemoryInfo* mi);
int  memory_info_update(MemoryInfo* mi);
void memory_info_print(const MemoryInfo* mi);
void memory_draw_bar(double percent, int width);
void memory_draw_graph(const MemoryInfo* mi, int width);
#endif
