#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

#include "core/cpu.h"
#include "core/memory.h"
#include "core/process.h"
#include "core/network.h"
#include "core/anomaly.h"
#include "core/logger.h"
#include "gui/widgets.h"
#include "gui/hotkeys.h"
#include "web/httpserver.h"

#define VERSION       "v2.0"
#define REFRESH_MS    1000
#define TOP_N         15
#define NET_TOP_N     12
#define CPU_ALERT     85.0
#define RAM_ALERT     90.0

/* ── Global state ───────────────────────────────────────────────────── */
static CpuInfo       g_cpu;
static MemoryInfo    g_mem;
static ProcessList   g_procs;
static NetInfo       g_net;
static AnomalyReport g_anomaly;
static HotkeyState   g_keys;
static volatile int  g_running = 1;
static char          g_statusMsg[256] = "";

/* ── Ctrl+C handler ─────────────────────────────────────────────────── */
static BOOL WINAPI ctrl_handler(DWORD type) {
    if(type==CTRL_C_EVENT||type==CTRL_CLOSE_EVENT) {
        g_running=0;
        widget_reset_color();
        printf("\n\nArrêt de SysWatch Pro...\n");
        return TRUE;
    }
    return FALSE;
}

/* ── Sort helpers ───────────────────────────────────────────────────── */
static int cmp_cpu(const void* a,const void* b){
    const ProcessInfo* pa=(const ProcessInfo*)a;
    const ProcessInfo* pb=(const ProcessInfo*)b;
    return (pb->cpuUsage>pa->cpuUsage)-(pb->cpuUsage<pa->cpuUsage);
}
static int cmp_mem(const void* a,const void* b){
    const ProcessInfo* pa=(const ProcessInfo*)a;
    const ProcessInfo* pb=(const ProcessInfo*)b;
    return (pb->memUsageKB>pa->memUsageKB)-(pb->memUsageKB<pa->memUsageKB);
}

/* ── Render process table ────────────────────────────────────────────── */
static void render_procs(int sortByCpu, const char* filter) {
    ProcessInfo sorted[MAX_PROCESSES];
    int cnt=g_procs.count;
    memcpy(sorted,g_procs.list,cnt*sizeof(ProcessInfo));
    qsort(sorted,cnt,sizeof(ProcessInfo),sortByCpu?cmp_cpu:cmp_mem);

    widget_set_color(8);
    printf("  %-6s  %-6s  %-10s  %-7s  %-24s\n",
           "PID","CPU%","RAM(KB)","Threads","Nom");
    printf("  %-6s  %-6s  %-10s  %-7s  %-24s\n",
           "------","------","----------","-------","------------------------");
    widget_reset_color();

    int shown=0;
    for(int i=0;i<cnt&&shown<TOP_N;i++){
        const ProcessInfo* p=&sorted[i];
        /* filter */
        if(filter&&filter[0]){
            char lo[64],f2[64]; int k=0;
            while(p->name[k]&&k<63){lo[k]=(char)tolower((unsigned char)p->name[k]);k++;}
            lo[k]='\0'; k=0;
            while(filter[k]&&k<63){f2[k]=(char)tolower((unsigned char)filter[k]);k++;}
            f2[k]='\0';
            if(!strstr(lo,f2)) continue;
        }
        if(p->suspicious)        widget_set_color(12);
        else if(p->anomalyScore>0) widget_set_color(14);
        else if(p->cpuUsage>50.0||p->memUsageKB>300000) widget_set_color(11);
        else widget_set_color(15);

        printf("  %-6lu  %5.1f%%  %10zu  %-7lu  %s",
               (unsigned long)p->pid, p->cpuUsage,
               p->memUsageKB, (unsigned long)p->threadCount, p->name);
        if(p->suspicious){ widget_set_color(12); printf("  [SUSPECT]"); }
        if(p->isNew){ widget_set_color(14); printf("  [NEW]"); }
        widget_reset_color(); printf("\n");

        if(p->fullPath[0]){
            widget_set_color(8);
            printf("           %s\n",p->fullPath);
            widget_reset_color();
        }
        shown++;
    }
}

/* ── Main render ─────────────────────────────────────────────────────── */
static void render(ViewMode mode, int sortByCpu) {
    widget_clear_screen();

    char modeStr[32];
    const char* modes[]={"CPU","RAM","Processus","Reseau","Anomalies","Logs","Tout"};
    snprintf(modeStr,sizeof(modeStr),"%s [F7=Tout]",modes[mode<7?mode:6]);
    widget_print_header(VERSION, modeStr);

    /* ── Global alerts ─────────────────────────────────────────────── */
    if(g_cpu.totalUsage>=CPU_ALERT){
        char buf[64]; snprintf(buf,sizeof(buf),"CPU CRITIQUE : %.1f%%",g_cpu.totalUsage);
        widget_print_alert(buf,3);
    }
    if(g_mem.usagePercent>=RAM_ALERT){
        char buf[64]; snprintf(buf,sizeof(buf),"RAM CRITIQUE : %.1f%%",g_mem.usagePercent);
        widget_print_alert(buf,3);
    }
    if(g_anomaly.critCount>0){
        char buf[64]; snprintf(buf,sizeof(buf),"%d processus CRITIQUES detectes par l'IA !",g_anomaly.critCount);
        widget_print_alert(buf,3);
    }
    if(g_net.suspiciousCount>0){
        char buf[64]; snprintf(buf,sizeof(buf),"%d connexion(s) reseau suspecte(s) !",g_net.suspiciousCount);
        widget_print_alert(buf,2);
    }

    /* ── CPU ─────────────────────────────────────────────────────────── */
    if(mode==VIEW_ALL||mode==VIEW_CPU){
        widget_print_section("PROCESSEUR (CPU)",11);
        cpu_info_print(&g_cpu);
    }

    /* ── RAM ─────────────────────────────────────────────────────────── */
    if(mode==VIEW_ALL||mode==VIEW_RAM){
        widget_print_section("MEMOIRE (RAM)",9);
        memory_info_print(&g_mem);
    }

    /* ── Processes ───────────────────────────────────────────────────── */
    if(mode==VIEW_ALL||mode==VIEW_PROCS){
        char title[64];
        snprintf(title,sizeof(title),"TOP PROCESSUS par %s  [F8=Basculer tri]",
                 sortByCpu?"CPU":"RAM");
        widget_print_section(title,14);
        render_procs(sortByCpu, g_keys.filterName);
    }

    /* ── Network ─────────────────────────────────────────────────────── */
    if(mode==VIEW_ALL||mode==VIEW_NETWORK){
        widget_print_section("RESEAU - Connexions actives",13);
        network_info_print(&g_net, NET_TOP_N);
    }

    /* ── AI Anomalies ────────────────────────────────────────────────── */
    if(mode==VIEW_ALL||mode==VIEW_ANOMALY){
        widget_print_section("IA - DETECTION D'ANOMALIES",12);
        anomaly_report_print(&g_anomaly);
    }

    /* ── Logs view ───────────────────────────────────────────────────── */
    if(mode==VIEW_LOGS){
        widget_print_section("LOGS FORENSIC CHIFFRES (dernieres lignes)",11);
        widget_set_color(8);
        printf("  Fichier: syswatch_forensic.log\n");
        printf("  Pour dechiffrer: SysWatchPro.exe --decrypt syswatch_forensic.log\n");
        widget_reset_color();
    }

    /* ── Dashboard web ───────────────────────────────────────────────── */
    widget_set_color(8);
    printf("\n  Dashboard web: ");
    widget_set_color(11);
    printf("http://localhost:%d",HTTP_PORT);
    widget_reset_color();
    printf("  (ouvrir dans navigateur)\n");

    /* ── Footer ──────────────────────────────────────────────────────── */
    int totalAnom=g_anomaly.warnCount+g_anomaly.alertCount+g_anomaly.critCount;
    widget_print_footer(g_procs.count,
                        /* count suspicious procs */
                        (int)(g_procs.count>0?g_anomaly.critCount:0),
                        totalAnom,
                        g_net.suspiciousCount,
                        g_statusMsg[0]?g_statusMsg:NULL);
    g_statusMsg[0]='\0';
}

/* ── Entry point ─────────────────────────────────────────────────────── */
int main(int argc, char* argv[]) {
    SetConsoleOutputCP(65001);
    SetConsoleCtrlHandler(ctrl_handler,TRUE);

    /* decrypt mode */
    if(argc>=3 && strcmp(argv[1],"--decrypt")==0) {
        logger_dump_decrypted(argv[2]);
        return 0;
    }

    /* resize console */
    HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if(GetConsoleScreenBufferInfo(hOut,&csbi)){
        COORD sz={130,3000}; SetConsoleScreenBufferSize(hOut,sz);
        SMALL_RECT r={0,0,129,55}; SetConsoleWindowInfo(hOut,TRUE,&r);
    }
    widget_set_title("SysWatch Pro v2.0");

    /* ── Init all modules ─────────────────────────────────────────── */
    widget_set_color(11);
    printf("\n  SysWatch Pro v2.0 - Demarrage...\n");
    widget_reset_color();

    cpu_info_init(&g_cpu);
    memory_info_init(&g_mem);
    process_list_init(&g_procs);
    network_info_init(&g_net);
    hotkeys_init(&g_keys);
    logger_init();

    /* ── HTTP Dashboard ────────────────────────────────────────────── */
    static HttpServerState httpState;
    httpState.cpu=&g_cpu; httpState.mem=&g_mem;
    httpState.procs=&g_procs; httpState.net=&g_net;
    httpState.anomaly=&g_anomaly;
    if(http_server_start(&httpState)){
        widget_set_color(10);
        printf("  Dashboard: http://localhost:%d\n",HTTP_PORT);
        widget_reset_color();
    } else {
        widget_set_color(8);
        printf("  Dashboard web non disponible (port %d occupe?)\n",HTTP_PORT);
        widget_reset_color();
    }

    logger_log_event("STARTUP","SysWatch Pro demarre");
    Sleep(800);

    /* ── Main loop ─────────────────────────────────────────────────── */
    while(g_running && !g_keys.quitRequested) {
        hotkeys_poll(&g_keys);

        /* handle kill request */
        if(g_keys.killRequested) {
            g_keys.killRequested=0;
            DWORD pid=g_keys.killPid;
            /* find name */
            char name[MAX_NAME_LEN]="?";
            for(int i=0;i<g_procs.count;i++)
                if(g_procs.list[i].pid==pid){ strncpy(name,g_procs.list[i].name,MAX_NAME_LEN-1); break; }
            int ok=process_kill(pid);
            logger_log_kill(pid,name,ok);
            snprintf(g_statusMsg,sizeof(g_statusMsg),
                     "KILL PID %lu (%s): %s",
                     (unsigned long)pid, name, ok?"SUCCES":"ECHEC (droits?)");
        }

        /* sync hotkey status msg */
        if(g_keys.statusMsg[0]) {
            strncpy(g_statusMsg,g_keys.statusMsg,sizeof(g_statusMsg)-1);
            g_keys.statusMsg[0]='\0';
        }

        /* update all data */
        cpu_info_update(&g_cpu);
        memory_info_update(&g_mem);
        process_list_update(&g_procs);

        /* build proc name/pid arrays for network */
        char   netNames[MAX_PROCESSES][64];
        DWORD  netPids[MAX_PROCESSES];
        for(int i=0;i<g_procs.count&&i<MAX_PROCESSES;i++){
            strncpy(netNames[i],g_procs.list[i].name,63);
            netPids[i]=g_procs.list[i].pid;
        }
        network_info_update(&g_net,(const char(*)[64])netNames,netPids,g_procs.count);

        /* IA analysis */
        anomaly_analyze(&g_anomaly,&g_procs,&g_net);

        /* log critical anomalies */
        for(int i=0;i<g_anomaly.count;i++){
            if(g_anomaly.results[i].level>=2)
                logger_log_anomaly(&g_anomaly.results[i]);
        }
        /* log suspicious net connections */
        for(int i=0;i<g_net.count;i++){
            if(g_net.list[i].suspicious)
                logger_log_net_alert(&g_net.list[i]);
        }
        logger_flush();

        /* alerts beep */
        static int cpuBeepActive=0, ramBeepActive=0;
        if(g_cpu.totalUsage>=CPU_ALERT && !cpuBeepActive){
            widget_beep(g_cpu.totalUsage>=95.0); cpuBeepActive=1;
        } else if(g_cpu.totalUsage<CPU_ALERT) cpuBeepActive=0;
        if(g_mem.usagePercent>=RAM_ALERT && !ramBeepActive){
            widget_beep(g_mem.usagePercent>=95.0); ramBeepActive=1;
        } else if(g_mem.usagePercent<RAM_ALERT) ramBeepActive=0;

        /* inject anomaly scores into process list for coloring */
        for(int i=0;i<g_anomaly.count;i++){
            for(int j=0;j<g_procs.count;j++){
                if(g_procs.list[j].pid==g_anomaly.results[i].pid){
                    g_procs.list[j].anomalyScore=g_anomaly.results[i].score;
                    break;
                }
            }
        }

        render(g_keys.mode, g_keys.sortByCpu);
        Sleep(REFRESH_MS);
    }

    /* ── Shutdown ────────────────────────────────────────────────────── */
    http_server_stop(&httpState);
    logger_log_event("SHUTDOWN","SysWatch Pro arrêté proprement");
    logger_close();
    widget_reset_color();
    printf("\n  SysWatch Pro arrêté. Logs: %s\n",LOG_FILE);
    return 0;
}
