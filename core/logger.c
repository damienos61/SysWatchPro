#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <windows.h>

static FILE* g_logFile = NULL;

/* XOR encrypt a buffer in-place */
static void xor_buf(unsigned char* buf, size_t len) {
    for(size_t i=0;i<len;i++) buf[i]^=LOG_XOR_KEY;
}

static void get_timestamp(char* buf, size_t sz) {
    time_t now=time(NULL);
    struct tm* t=localtime(&now);
    strftime(buf,sz,"%Y-%m-%d %H:%M:%S",t);
}

static long file_size(FILE* f) {
    long cur=ftell(f);
    fseek(f,0,SEEK_END);
    long sz=ftell(f);
    fseek(f,cur,SEEK_SET);
    return sz;
}

static void rotate_if_needed(void) {
    if(!g_logFile) return;
    if(file_size(g_logFile)>=LOG_MAX_SIZE) {
        fclose(g_logFile);
        /* rename old */
        char rotated[64];
        time_t now=time(NULL);
        snprintf(rotated,sizeof(rotated),"syswatch_%ld.log.old",(long)now);
        rename(LOG_FILE,rotated);
        g_logFile=fopen(LOG_FILE,"wb");
    }
}

int logger_init(void) {
    g_logFile=fopen(LOG_FILE,"ab");
    if(!g_logFile) return 0;
    /* write a session start marker */
    char ts[32]; get_timestamp(ts,sizeof(ts));
    char header[128];
    snprintf(header,sizeof(header),
             "\n[SESSION_START] %s  SysWatchPro v2.0\n",ts);
    unsigned char buf[256];
    memcpy(buf,header,strlen(header));
    xor_buf(buf,strlen(header));
    fwrite(buf,1,strlen(header),g_logFile);
    fflush(g_logFile);
    return 1;
}

static void write_encrypted(const char* line) {
    if(!g_logFile) return;
    rotate_if_needed();
    size_t len=strlen(line);
    unsigned char* buf=(unsigned char*)malloc(len+2);
    if(!buf) return;
    memcpy(buf,line,len);
    buf[len]='\n'; buf[len+1]='\0';
    xor_buf(buf,len+1);
    fwrite(buf,1,len+1,g_logFile);
    free(buf);
}

void logger_log_anomaly(const AnomalyResult* r) {
    char ts[32]; get_timestamp(ts,sizeof(ts));
    char line[512];
    const char* levels[]={"OK","WARN","ALERTE","CRITIQUE"};
    snprintf(line,sizeof(line),
             "[ANOMALY] %s  PID=%-6lu  Score=%3d  Level=%-8s  Name=%-24s  %s",
             ts,(unsigned long)r->pid,r->score,
             r->level<4?levels[r->level]:"?",
             r->name, r->reasons);
    write_encrypted(line);
}

void logger_log_kill(DWORD pid, const char* name, int success) {
    char ts[32]; get_timestamp(ts,sizeof(ts));
    char line[256];
    snprintf(line,sizeof(line),
             "[KILL]    %s  PID=%-6lu  Name=%-24s  Result=%s",
             ts,(unsigned long)pid,name,success?"SUCCESS":"FAILED");
    write_encrypted(line);
    logger_flush();
}

void logger_log_net_alert(const NetConnection* c) {
    char ts[32]; get_timestamp(ts,sizeof(ts));
    char line[512];
    snprintf(line,sizeof(line),
             "[NET]     %s  PID=%-6lu  Proc=%-16s  %s:%-5u -> %s:%-5u  %s",
             ts,(unsigned long)c->pid,c->procName,
             c->localAddr,c->localPort,
             c->remoteAddr,c->remotePort,
             c->state);
    write_encrypted(line);
}

void logger_log_event(const char* category, const char* msg) {
    char ts[32]; get_timestamp(ts,sizeof(ts));
    char line[512];
    snprintf(line,sizeof(line),"[%-8s] %s  %s",category,ts,msg);
    write_encrypted(line);
}

void logger_flush(void) { if(g_logFile) fflush(g_logFile); }

void logger_close(void) {
    if(g_logFile) {
        logger_log_event("SESSION","SESSION_END");
        fclose(g_logFile);
        g_logFile=NULL;
    }
}

/* Decrypt and dump log to stdout */
void logger_dump_decrypted(const char* filename) {
    FILE* f=fopen(filename,"rb");
    if(!f){ printf("Impossible d'ouvrir %s\n",filename); return; }
    unsigned char buf[256];
    size_t n;
    while((n=fread(buf,1,sizeof(buf),f))>0) {
        xor_buf(buf,n);
        fwrite(buf,1,n,stdout);
    }
    fclose(f);
}
