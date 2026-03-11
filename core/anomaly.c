#include "anomaly.h"
#include "../gui/widgets.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* CPU history per PID to detect sustained high CPU */
#define CPU_TRACK 256
static DWORD  cpuPids[CPU_TRACK];
static int    cpuHighCount[CPU_TRACK];  /* consecutive high-CPU ticks */
static int    cpuTrackN = 0;

static int get_cpu_streak(DWORD pid, double cpu) {
    for(int i=0;i<cpuTrackN;i++) {
        if(cpuPids[i]==pid) {
            if(cpu>80.0) cpuHighCount[i]++;
            else         cpuHighCount[i]=0;
            return cpuHighCount[i];
        }
    }
    if(cpuTrackN<CPU_TRACK) {
        cpuPids[cpuTrackN]=pid;
        cpuHighCount[cpuTrackN]=(cpu>80.0)?1:0;
        cpuTrackN++;
        return cpuHighCount[cpuTrackN-1];
    }
    return 0;
}

static int pid_has_suspicious_conn(DWORD pid, const NetInfo* net) {
    for(int i=0;i<net->count;i++)
        if(net->list[i].pid==pid && net->list[i].suspicious) return 1;
    return 0;
}

static int pid_has_any_conn(DWORD pid, const NetInfo* net) {
    for(int i=0;i<net->count;i++)
        if(net->list[i].pid==pid) return 1;
    return 0;
}

void anomaly_analyze(AnomalyReport* report,
                     ProcessList*   procs,
                     NetInfo*       net) {
    memset(report,0,sizeof(*report));

    for(int i=0;i<procs->count;i++) {
        const ProcessInfo* p=&procs->list[i];
        int score=0;
        char reasons[256]="";

        /* ── Known malware name ─────────────────────────────── */
        if(p->suspicious) {
            score+=SCORE_KNOWN_MALWARE;
            strncat(reasons,"[NOM MALVEILLANT] ",sizeof(reasons)-strlen(reasons)-1);
        }

        /* ── Path anomaly (system binary outside system32) ───── */
        if(p->fullPath[0]) {
            char loPath[MAX_PATH_LEN],loName[MAX_NAME_LEN];
            int k=0;
            while(p->fullPath[k]&&k<MAX_PATH_LEN-1){loPath[k]=(char)tolower((unsigned char)p->fullPath[k]);k++;}
            loPath[k]='\0'; k=0;
            while(p->name[k]&&k<MAX_NAME_LEN-1){loName[k]=(char)tolower((unsigned char)p->name[k]);k++;}
            loName[k]='\0';
            const char* sysNames[]={"svchost.exe","lsass.exe","csrss.exe","rundll32.exe",NULL};
            for(int j=0;sysNames[j];j++) {
                if(strcmp(loName,sysNames[j])==0)
                    if(!strstr(loPath,"system32")&&!strstr(loPath,"syswow64")) {
                        score+=SCORE_PATH_ANOMALY;
                        strncat(reasons,"[CHEMIN ANORMAL] ",sizeof(reasons)-strlen(reasons)-1);
                    }
            }
        }

        /* ── Sustained high CPU ─────────────────────────────── */
        int streak=get_cpu_streak(p->pid,p->cpuUsage);
        if(streak>=10) { /* 10 consecutive seconds > 80% */
            score+=SCORE_CPU_HIGH;
            strncat(reasons,"[CPU ELEVE 10s+] ",sizeof(reasons)-strlen(reasons)-1);
        }

        /* ── New process ────────────────────────────────────── */
        if(p->isNew && p->pid>4) {
            score+=SCORE_NEW_PROCESS;
            strncat(reasons,"[NOUVEAU PROCESSUS] ",sizeof(reasons)-strlen(reasons)-1);
            /* new + high memory = extra suspicious */
            if(p->memUsageKB>100000) {
                score+=SCORE_HIGH_MEM_NEW;
                strncat(reasons,"[MEMOIRE ELEVEE] ",sizeof(reasons)-strlen(reasons)-1);
            }
        }

        /* ── Suspicious network connection ──────────────────── */
        if(pid_has_suspicious_conn(p->pid,net)) {
            score+=SCORE_NET_SUSPECT;
            strncat(reasons,"[CONNEXION SUSPECTE] ",sizeof(reasons)-strlen(reasons)-1);
        }

        /* ── Thread burst ───────────────────────────────────── */
        if(p->threadCount>100 && p->memUsageKB<50000 && p->pid>4) {
            score+=SCORE_THREAD_BURST;
            strncat(reasons,"[THREADS EXCESSIFS] ",sizeof(reasons)-strlen(reasons)-1);
        }

        /* ── Network activity for unknown small processes ────── */
        if(p->memUsageKB<5000 && p->memUsageKB>0 && pid_has_any_conn(p->pid,net) && p->pid>4) {
            score+=15;
            strncat(reasons,"[PETIT PROC+RESEAU] ",sizeof(reasons)-strlen(reasons)-1);
        }

        if(score<=0) continue;  /* clean processes not stored */
        if(score>100) score=100;

        AnomalyResult* r=&report->results[report->count];
        r->pid=p->pid;
        strncpy(r->name,p->name,63);
        r->score=score;
        strncpy(r->reasons,reasons,255);
        if     (score>=ANOMALY_CRIT)  { r->level=3; report->critCount++;  }
        else if(score>=ANOMALY_ALERT) { r->level=2; report->alertCount++; }
        else if(score>=ANOMALY_WARN)  { r->level=1; report->warnCount++;  }
        else                           { r->level=0; }
        report->count++;
    }
}

void anomaly_report_print(const AnomalyReport* report) {
    if(report->count==0) {
        widget_set_color(10);
        printf("  Aucune anomalie detectee. Systeme sain.\n");
        widget_reset_color();
        return;
    }
    widget_set_color(8);
    printf("  %-6s  %-5s  %-24s  %s\n","PID","Score","Nom","Raisons");
    printf("  %-6s  %-5s  %-24s  %s\n","------","-----","------------------------","----------------------------");
    widget_reset_color();

    for(int i=0;i<report->count;i++) {
        const AnomalyResult* r=&report->results[i];
        if(r->level==0) continue;
        switch(r->level){
            case 3: widget_set_color(12); break;
            case 2: widget_set_color(14); break;
            default: widget_set_color(11); break;
        }
        const char* lbl[]={"OK","WARN","ALERTE","CRITIQUE"};
        printf("  %-6lu  [%3d]  %-24s  %s  (%s)\n",
               (unsigned long)r->pid, r->score, r->name,
               lbl[r->level], r->reasons);
        widget_reset_color();
    }
    printf("\n");
    widget_set_color(8);
    printf("  WARN:%d  ALERTE:%d  ",report->warnCount,report->alertCount);
    widget_reset_color();
    if(report->critCount>0){ widget_set_color(12); printf("CRITIQUE:%d [!!!]",report->critCount); }
    else{ widget_set_color(10); printf("CRITIQUE:0 [OK]"); }
    widget_reset_color(); printf("\n");
}
