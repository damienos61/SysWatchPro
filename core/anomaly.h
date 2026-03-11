#ifndef ANOMALY_H
#define ANOMALY_H

#include "process.h"
#include "network.h"

/* Anomaly score weights */
#define SCORE_CPU_HIGH        30   /* CPU > 80% sustained */
#define SCORE_NET_SUSPECT     25   /* outbound to suspicious port */
#define SCORE_NEW_PROCESS     20   /* appeared < 60s ago */
#define SCORE_PATH_ANOMALY    35   /* system binary outside system32 */
#define SCORE_THREAD_BURST    15   /* threads > 100 for small process */
#define SCORE_KNOWN_MALWARE   50   /* name matches known bad list */
#define SCORE_HIGH_MEM_NEW    20   /* new process using > 100MB */

#define ANOMALY_WARN   40
#define ANOMALY_ALERT  65
#define ANOMALY_CRIT   85

typedef struct {
    DWORD  pid;
    char   name[64];
    int    score;
    char   reasons[256];
    int    level;   /* 0=ok 1=warn 2=alert 3=critical */
} AnomalyResult;

typedef struct {
    AnomalyResult results[MAX_PROCESSES];
    int           count;
    int           warnCount;
    int           alertCount;
    int           critCount;
} AnomalyReport;

void anomaly_analyze(AnomalyReport* report,
                     ProcessList*   procs,
                     NetInfo*       net);
void anomaly_report_print(const AnomalyReport* report);

#endif
