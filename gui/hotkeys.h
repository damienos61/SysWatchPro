#ifndef HOTKEYS_H
#define HOTKEYS_H

#include <windows.h>

/* View modes toggled by F-keys */
typedef enum {
    VIEW_CPU     = 0,
    VIEW_RAM     = 1,
    VIEW_PROCS   = 2,
    VIEW_NETWORK = 3,
    VIEW_ANOMALY = 4,
    VIEW_LOGS    = 5,
    VIEW_ALL     = 6
} ViewMode;

typedef struct {
    ViewMode  mode;
    int       killRequested;
    DWORD     killPid;
    int       quitRequested;
    int       sortByCpu;       /* 1=cpu, 0=ram */
    char      filterName[64];
    char      statusMsg[256];
} HotkeyState;

void hotkeys_init(HotkeyState* hs);
void hotkeys_poll(HotkeyState* hs);  /* non-blocking check */

#endif
