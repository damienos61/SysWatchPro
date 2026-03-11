#ifndef CPU_H
#define CPU_H
#include <windows.h>
#define HISTORY_LEN 60

typedef struct {
    double totalUsage;
    int    coreCount;
    double history[HISTORY_LEN];
    int    histIdx;
    int    histFilled;
} CpuInfo;

void cpu_info_init(CpuInfo* ci);
int  cpu_info_update(CpuInfo* ci);
void cpu_draw_bar(double percent, int width);
void cpu_draw_graph(const CpuInfo* ci, int width);
void cpu_info_print(const CpuInfo* ci);
#endif
