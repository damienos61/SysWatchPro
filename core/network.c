#include "network.h"
#include "../gui/widgets.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Manual declarations for old MinGW */
#ifndef TCP_TABLE_OWNER_PID_ALL
#define TCP_TABLE_OWNER_PID_ALL 5
#endif

typedef struct {
    DWORD dwOwningPid;
    DWORD dwLocalAddr;
    DWORD dwLocalPort;
    DWORD dwRemoteAddr;
    DWORD dwRemotePort;
    DWORD dwState;
} MIB_TCPROW_OWNER_PID_COMPAT;

typedef struct {
    DWORD dwNumEntries;
    MIB_TCPROW_OWNER_PID_COMPAT table[1];
} MIB_TCPTABLE_OWNER_PID_COMPAT;

typedef struct {
    DWORD dwOwningPid;
    DWORD dwLocalAddr;
    DWORD dwLocalPort;
} MIB_UDPROW_OWNER_PID_COMPAT;

typedef struct {
    DWORD dwNumEntries;
    MIB_UDPROW_OWNER_PID_COMPAT table[1];
} MIB_UDPTABLE_OWNER_PID_COMPAT;

#define UDP_TABLE_OWNER_PID_COMPAT 1

DWORD WINAPI GetExtendedTcpTable(PVOID,PDWORD,BOOL,ULONG,int,ULONG);

/* TCP states */
static const char* tcp_state_str(DWORD s){
    switch(s){
        case 1:  return "CLOSED";
        case 2:  return "LISTEN";
        case 3:  return "SYN_SENT";
        case 4:  return "SYN_RCVD";
        case 5:  return "ESTABLISHED";
        case 6:  return "FIN_WAIT1";
        case 7:  return "FIN_WAIT2";
        case 8:  return "CLOSE_WAIT";
        case 9:  return "CLOSING";
        case 10: return "LAST_ACK";
        case 11: return "TIME_WAIT";
        case 12: return "DELETE_TCB";
        default: return "UNKNOWN";
    }
}

/* ports considered suspicious for outbound connections */
static int is_suspicious_port(USHORT port){
    /* common RAT/miner ports */
    static const USHORT bad[]={
        4444,5555,6666,7777,8888,9999,1337,31337,
        3389, /* RDP outbound */
        4545,5050,6060,
        14444,14433, /* monero miners */
        3333,5004,
        0
    };
    for(int i=0;bad[i];i++) if(port==bad[i]) return 1;
    return 0;
}

static int is_private_ip(const char* ip){
    /* 10.x, 172.16-31.x, 192.168.x, 127.x, 0.0.0.0 */
    if(strncmp(ip,"10.",3)==0)    return 1;
    if(strncmp(ip,"192.168.",8)==0) return 1;
    if(strncmp(ip,"127.",4)==0)   return 1;
    if(strncmp(ip,"0.0.0.0",7)==0) return 1;
    if(strncmp(ip,"172.",4)==0){
        int b=0; sscanf(ip+4,"%d",&b);
        if(b>=16&&b<=31) return 1;
    }
    return 0;
}

static void find_proc_name(DWORD pid, const char procNames[][64],
                            const DWORD* pids, int n, char* out){
    for(int i=0;i<n;i++) if(pids[i]==pid){ strncpy(out,procNames[i],63); return; }
    strcpy(out,"?");
}

int network_info_init(NetInfo* ni){ memset(ni,0,sizeof(*ni)); return 1; }

int network_info_update(NetInfo* ni,
                         const char procNames[][64],
                         const DWORD* pids, int procCount){
    memset(ni,0,sizeof(*ni));

    /* ── TCP connections ─────────────────────────────────────── */
    ULONG size=0;
    GetExtendedTcpTable(NULL,&size,FALSE,AF_INET,TCP_TABLE_OWNER_PID_ALL,0);
    MIB_TCPTABLE_OWNER_PID_COMPAT* tcpTable=(MIB_TCPTABLE_OWNER_PID_COMPAT*)malloc(size);
    if(tcpTable && GetExtendedTcpTable(tcpTable,&size,FALSE,AF_INET,TCP_TABLE_OWNER_PID_ALL,0)==NO_ERROR){
        for(DWORD i=0;i<tcpTable->dwNumEntries&&ni->count<MAX_CONNECTIONS;i++){
            MIB_TCPROW_OWNER_PID_COMPAT* row=&tcpTable->table[i];
            NetConnection* c=&ni->list[ni->count];
            c->pid=row->dwOwningPid;
            find_proc_name(c->pid,procNames,pids,procCount,c->procName);
            struct in_addr la,ra;
            la.s_addr=row->dwLocalAddr; ra.s_addr=row->dwRemoteAddr;
            strncpy(c->localAddr,inet_ntoa(la),45);
            strncpy(c->remoteAddr,inet_ntoa(ra),45);
            c->localPort=ntohs((USHORT)row->dwLocalPort);
            c->remotePort=ntohs((USHORT)row->dwRemotePort);
            strncpy(c->state,tcp_state_str(row->dwState),23);
            c->suspicious=(!is_private_ip(c->remoteAddr)&&c->remoteAddr[0]!='0')
                          && is_suspicious_port(c->remotePort);
            if(c->suspicious) ni->suspiciousCount++;
            ni->count++; ni->totalTCP++;
        }
    }
    if(tcpTable) free(tcpTable);

    ni->totalUDP = 0; /* UDP non disponible sur ce compilateur */
    return ni->count;
}

void network_info_print(const NetInfo* ni, int topN){
    widget_set_color(8);
    printf("  %-6s  %-16s  %-6s  %-16s  %-6s  %-12s  %s\n",
           "PID","Local IP","L.Port","Remote IP","R.Port","Etat","Process");
    printf("  %-6s  %-16s  %-6s  %-16s  %-6s  %-12s  %s\n",
           "------","----------------","------","----------------","------","------------","-------");
    widget_reset_color();

    int shown=0;
    for(int i=0;i<ni->count&&shown<topN;i++){
        const NetConnection* c=&ni->list[i];
        if(strcmp(c->state,"ESTABLISHED")!=0 && strcmp(c->state,"LISTEN")!=0
           && strcmp(c->state,"UDP")!=0) continue;
        if(c->suspicious) widget_set_color(12);
        else if(strcmp(c->state,"LISTEN")==0||strcmp(c->state,"UDP")==0)
            widget_set_color(14);
        else widget_set_color(15);
        printf("  %-6lu  %-16s  %-6u  %-16s  %-6u  %-12s  %s%s\n",
               (unsigned long)c->pid,
               c->localAddr, c->localPort,
               c->remoteAddr, c->remotePort,
               c->state, c->procName,
               c->suspicious?" [SUSPECT]":"");
        widget_reset_color();
        shown++;
    }
    printf("\n");
    widget_set_color(11);
    printf("  TCP: %d  UDP: %d  Total: %d",ni->totalTCP,ni->totalUDP,ni->count);
    if(ni->suspiciousCount>0){ widget_set_color(12); printf("  Connexions suspectes: %d [!!!]",ni->suspiciousCount); }
    widget_reset_color(); printf("\n");
}
