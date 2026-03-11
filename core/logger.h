#ifndef LOGGER_H
#define LOGGER_H

#include "anomaly.h"
#include "network.h"

/* XOR key for log obfuscation (simple forensic protection) */
#define LOG_XOR_KEY  0xA7
#define LOG_FILE     "syswatch_forensic.log"
#define LOG_MAX_SIZE (5*1024*1024)  /* 5 MB, then rotate */

int  logger_init(void);
void logger_log_anomaly(const AnomalyResult* r);
void logger_log_kill(DWORD pid, const char* name, int success);
void logger_log_net_alert(const NetConnection* c);
void logger_log_event(const char* category, const char* msg);
void logger_flush(void);
void logger_close(void);

/* Read & decrypt log to stdout */
void logger_dump_decrypted(const char* filename);

#endif
