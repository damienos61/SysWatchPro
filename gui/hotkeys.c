#include "hotkeys.h"
#include "widgets.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>

void hotkeys_init(HotkeyState* hs){
    memset(hs,0,sizeof(*hs));
    hs->mode=VIEW_ALL;
    hs->sortByCpu=1;
}

void hotkeys_poll(HotkeyState* hs){
    HANDLE hIn=GetStdHandle(STD_INPUT_HANDLE);
    DWORD n=0;
    GetNumberOfConsoleInputEvents(hIn,&n);
    if(n==0) return;

    INPUT_RECORD ir[32];
    DWORD read=0;
    ReadConsoleInput(hIn,ir,32,&read);

    for(DWORD i=0;i<read;i++){
        if(ir[i].EventType!=KEY_EVENT) continue;
        if(!ir[i].Event.KeyEvent.bKeyDown) continue;
        WORD vk=ir[i].Event.KeyEvent.wVirtualKeyCode;

        switch(vk){
            case VK_F1:
                hs->mode=VIEW_CPU;
                snprintf(hs->statusMsg,sizeof(hs->statusMsg),"Vue: CPU");
                break;
            case VK_F2:
                hs->mode=VIEW_RAM;
                snprintf(hs->statusMsg,sizeof(hs->statusMsg),"Vue: Memoire");
                break;
            case VK_F3:
                hs->mode=VIEW_PROCS;
                snprintf(hs->statusMsg,sizeof(hs->statusMsg),"Vue: Processus");
                break;
            case VK_F4:
                hs->mode=VIEW_NETWORK;
                snprintf(hs->statusMsg,sizeof(hs->statusMsg),"Vue: Reseau");
                break;
            case VK_F5:
                hs->mode=VIEW_ANOMALY;
                snprintf(hs->statusMsg,sizeof(hs->statusMsg),"Vue: IA Anomalies");
                break;
            case VK_F6:
                hs->mode=VIEW_LOGS;
                snprintf(hs->statusMsg,sizeof(hs->statusMsg),"Vue: Logs forensic");
                break;
            case VK_F7:
                hs->mode=VIEW_ALL;
                snprintf(hs->statusMsg,sizeof(hs->statusMsg),"Vue: Tout afficher");
                break;
            case VK_F8:
                hs->sortByCpu=!hs->sortByCpu;
                snprintf(hs->statusMsg,sizeof(hs->statusMsg),
                         "Tri: %s",hs->sortByCpu?"CPU":"RAM");
                break;
            case VK_F9:
                /* prompt for PID */
                {
                    COORD pos={0,50};
                    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),pos);
                    widget_set_color(14);
                    printf("  Entrez PID a tuer: ");
                    widget_reset_color();
                    DWORD pid=0;
                    if(scanf("%lu",&pid)==1 && pid>4){
                        hs->killPid=pid;
                        hs->killRequested=1;
                    }
                    FlushConsoleInputBuffer(hIn);
                }
                break;
            case VK_F10:
                hs->quitRequested=1;
                break;
            default: break;
        }
    }
}
