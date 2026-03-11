#include "httpserver.h"
#include "dashboard.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

static HttpServerState* g_state = NULL;

/* ── JSON builders ─────────────────────────────────────────────────── */

static void build_json(char* buf, size_t sz) {
    HttpServerState* s = g_state;
    if(!s) { snprintf(buf,sz,"{}"); return; }

    char tmp[65536];
    int  pos = 0;

    /* root open */
    pos += snprintf(tmp+pos, sizeof(tmp)-pos, "{");

    /* cpu */
    pos += snprintf(tmp+pos, sizeof(tmp)-pos,
        "\"cpu\":{\"usage\":%.1f,\"cores\":%d,\"history\":[",
        s->cpu->totalUsage, s->cpu->coreCount);
    int hlen = s->cpu->histFilled < HISTORY_LEN ? s->cpu->histFilled : HISTORY_LEN;
    for(int i=0;i<hlen;i++){
        int idx=(s->cpu->histIdx - hlen + i + HISTORY_LEN) % HISTORY_LEN;
        pos += snprintf(tmp+pos,sizeof(tmp)-pos,"%.1f%s",
                        s->cpu->history[idx], i<hlen-1?",":"");
    }
    pos += snprintf(tmp+pos,sizeof(tmp)-pos,"]},");

    /* mem */
    pos += snprintf(tmp+pos,sizeof(tmp)-pos,
        "\"mem\":{\"usage\":%.1f,\"used\":%lu,\"total\":%lu,\"history\":[",
        s->mem->usagePercent,
        (unsigned long)(s->mem->usedPhysKB*1024),
        (unsigned long)(s->mem->totalPhysKB*1024));
    int mlen = s->mem->histFilled < HISTORY_LEN ? s->mem->histFilled : HISTORY_LEN;
    for(int i=0;i<mlen;i++){
        int idx=(s->mem->histIdx - mlen + i + HISTORY_LEN) % HISTORY_LEN;
        pos += snprintf(tmp+pos,sizeof(tmp)-pos,"%.1f%s",
                        s->mem->history[idx], i<mlen-1?",":"");
    }
    pos += snprintf(tmp+pos,sizeof(tmp)-pos,"]},");

    /* anomalies */
    pos += snprintf(tmp+pos,sizeof(tmp)-pos,
        "\"anomalies\":{\"crit\":%d,\"alert\":%d,\"warn\":%d,\"list\":[",
        s->anomaly->critCount, s->anomaly->alertCount, s->anomaly->warnCount);
    for(int i=0;i<s->anomaly->count&&i<30;i++){
        const AnomalyResult* r=&s->anomaly->results[i];
        if(r->level==0) continue;
        /* escape name */
        char name[70]; strncpy(name,r->name,63); name[63]='\0';
        pos += snprintf(tmp+pos,sizeof(tmp)-pos,
            "{\"pid\":%lu,\"name\":\"%s\",\"score\":%d,\"level\":%d}%s",
            (unsigned long)r->pid, name, r->score, r->level,
            i<s->anomaly->count-1?",":"");
    }
    pos += snprintf(tmp+pos,sizeof(tmp)-pos,"]},");

    /* network */
    pos += snprintf(tmp+pos,sizeof(tmp)-pos,
        "\"network\":{\"tcp\":%d,\"udp\":%d,\"suspects\":%d,\"list\":[",
        s->net->totalTCP, s->net->totalUDP, s->net->suspiciousCount);
    int nshown=0;
    for(int i=0;i<s->net->count&&nshown<20;i++){
        const NetConnection* c=&s->net->list[i];
        if(strcmp(c->state,"ESTABLISHED")!=0&&strcmp(c->state,"LISTEN")!=0
           &&strcmp(c->state,"UDP")!=0) continue;
        pos += snprintf(tmp+pos,sizeof(tmp)-pos,
            "{\"pid\":%lu,\"proc\":\"%s\",\"local\":\"%s\",\"lport\":%u,"
            "\"remote\":\"%s\",\"rport\":%u,\"state\":\"%s\",\"suspicious\":%s}%s",
            (unsigned long)c->pid, c->procName,
            c->localAddr, c->localPort,
            c->remoteAddr, c->remotePort,
            c->state, c->suspicious?"true":"false",
            nshown<19?",":"");
        nshown++;
    }
    pos += snprintf(tmp+pos,sizeof(tmp)-pos,"]},");

    /* procs top 25 by cpu */
    pos += snprintf(tmp+pos,sizeof(tmp)-pos,"\"procs\":[");
    /* simple selection of top 25 */
    int shown=0;
    for(int i=0;i<s->procs->count&&shown<25;i++){
        const ProcessInfo* p=&s->procs->list[i];
        if(p->cpuUsage<0.1&&p->memUsageKB<10000) continue;
        pos += snprintf(tmp+pos,sizeof(tmp)-pos,
            "{\"pid\":%lu,\"name\":\"%s\",\"cpu\":%.1f,\"mem\":%lu,"
            "\"threads\":%lu,\"suspicious\":%s}%s",
            (unsigned long)p->pid, p->name,
            p->cpuUsage, (unsigned long)p->memUsageKB,
            (unsigned long)p->threadCount,
            p->suspicious?"true":"false",
            shown<24?",":"");
        shown++;
    }
    pos += snprintf(tmp+pos,sizeof(tmp)-pos,"]}");

    /* done */
    if(pos < (int)sz) { memcpy(buf,tmp,pos+1); }
    else { snprintf(buf,sz,"{\"error\":\"buffer overflow\"}"); }
}

/* ── Handle one HTTP client ─────────────────────────────────────────── */
static void handle_client(SOCKET client) {
    char req[1024];
    int n = recv(client, req, sizeof(req)-1, 0);
    if(n<=0){ closesocket(client); return; }
    req[n]='\0';

    char resp[131072];
    int  rlen=0;

    if(strncmp(req,"GET /api/data",13)==0) {
        char json[65536];
        build_json(json,sizeof(json));
        rlen = snprintf(resp,sizeof(resp),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Connection: close\r\n"
            "Content-Length: %d\r\n\r\n%s",
            (int)strlen(json), json);
    } else {
        /* serve dashboard */
        rlen = snprintf(resp,sizeof(resp),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Connection: close\r\n"
            "Content-Length: %d\r\n\r\n%s",
            (int)strlen(DASHBOARD_HTML), DASHBOARD_HTML);
    }
    send(client, resp, rlen, 0);
    closesocket(client);
}

/* ── Server thread ─────────────────────────────────────────────────── */
static DWORD WINAPI server_thread(LPVOID param) {
    HttpServerState* state=(HttpServerState*)param;

    WSADATA wsa; WSAStartup(MAKEWORD(2,2),&wsa);

    SOCKET srv=socket(AF_INET,SOCK_STREAM,0);
    if(srv==INVALID_SOCKET){ WSACleanup(); return 1; }

    int yes=1; setsockopt(srv,SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    addr.sin_port=htons(HTTP_PORT);

    if(bind(srv,(struct sockaddr*)&addr,sizeof(addr))!=0||listen(srv,8)!=0){
        closesocket(srv); WSACleanup(); return 1;
    }

    /* non-blocking accept */
    u_long mode=1; ioctlsocket(srv,FIONBIO,&mode);

    while(state->running){
        SOCKET client=accept(srv,NULL,NULL);
        if(client==INVALID_SOCKET){ Sleep(50); continue; }
        handle_client(client);
    }
    closesocket(srv);
    WSACleanup();
    return 0;
}

int http_server_start(HttpServerState* state) {
    g_state=state;
    state->running=1;
    HANDLE h=CreateThread(NULL,0,server_thread,state,0,NULL);
    return h!=NULL;
}

void http_server_stop(HttpServerState* state) {
    state->running=0;
}
