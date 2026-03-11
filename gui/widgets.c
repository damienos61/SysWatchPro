#include "widgets.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <windows.h>

void widget_set_color(int color){
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),(WORD)color);
}
void widget_reset_color(void){ widget_set_color(7); }

void widget_draw_bar(double percent, int width){
    int filled=(int)(percent*width/100.0);
    if(filled>width) filled=width;
    if(filled<0)     filled=0;
    putchar('[');
    for(int i=0;i<width;i++){
        if(i<filled){
            if     (percent<50.0) widget_set_color(10);
            else if(percent<80.0) widget_set_color(14);
            else                  widget_set_color(12);
            putchar('|');
        } else { widget_set_color(8); putchar('-'); }
    }
    widget_reset_color();
    printf("] ");
    if     (percent<50.0) widget_set_color(10);
    else if(percent<80.0) widget_set_color(14);
    else                  widget_set_color(12);
    printf("%5.1f%%", percent);
    widget_reset_color();
}

void widget_clear_screen(void){
    HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
    COORD c={0,0}; DWORD w;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if(!GetConsoleScreenBufferInfo(h,&csbi)){system("cls");return;}
    DWORD sz=csbi.dwSize.X*csbi.dwSize.Y;
    FillConsoleOutputCharacterA(h,' ',sz,c,&w);
    FillConsoleOutputAttribute(h,csbi.wAttributes,sz,c,&w);
    SetConsoleCursorPosition(h,c);
}

void widget_set_title(const char* title){ SetConsoleTitleA(title); }

void widget_print_header(const char* version, const char* mode){
    time_t now=time(NULL); struct tm* t=localtime(&now);
    char tb[32]; strftime(tb,sizeof(tb),"%Y-%m-%d  %H:%M:%S",t);
    widget_set_color(11);
    printf("  +===============================================================+\n");
    printf("  |  ");
    widget_set_color(15); printf("SysWatch Pro %-6s",version);
    widget_set_color(8);  printf(" | ");
    widget_set_color(14); printf("%s",tb);
    widget_set_color(8);  printf(" | ");
    widget_set_color(10); printf("%-10s",mode);
    widget_set_color(11); printf("  |\n");
    printf("  +===============================================================+\n");
    widget_reset_color();
}

void widget_print_section(const char* title, int color){
    printf("\n");
    widget_set_color(color);
    printf("  >> %-45s",title);
    widget_set_color(8);
    printf("----------\n");
    widget_reset_color();
}

void widget_print_separator(void){
    widget_set_color(8);
    printf("  "); for(int i=0;i<65;i++) putchar('-'); printf("\n");
    widget_reset_color();
}

void widget_print_alert(const char* msg, int level){
    switch(level){
        case 3: widget_set_color(12); printf("  [CRITIQUE] "); break;
        case 2: widget_set_color(14); printf("  [ALERTE]   "); break;
        default:widget_set_color(11); printf("  [INFO]     "); break;
    }
    printf("%s\n",msg);
    widget_reset_color();
}

void widget_print_footer(int procs, int suspects, int anomalies,
                          int netSuspects, const char* msg){
    widget_print_separator();
    printf("  ");
    widget_set_color(15); printf("Procs:");
    widget_set_color(11); printf("%d  ",procs);
    widget_set_color(15); printf("Suspects:");
    if(suspects>0){ widget_set_color(12); printf("%d  ",suspects); }
    else           { widget_set_color(10); printf("0  "); }
    widget_set_color(15); printf("Anomalies:");
    if(anomalies>0){ widget_set_color(14); printf("%d  ",anomalies); }
    else            { widget_set_color(10); printf("0  "); }
    widget_set_color(15); printf("Net:");
    if(netSuspects>0){ widget_set_color(12); printf("%d",netSuspects); }
    else              { widget_set_color(10); printf("0"); }
    widget_reset_color();
    printf("\n");
    widget_set_color(8);
    printf("  [F1]CPU  [F2]RAM  [F3]Procs  [F4]Reseau  [F5]Anomalies  [F6]Logs  [F9]Kill  [F10]Quitter\n");
    widget_reset_color();
    if(msg&&msg[0]){
        widget_set_color(13);
        printf("  %s\n",msg);
        widget_reset_color();
    }
}

void widget_beep(int critical){
    if(critical){ Beep(1400,120); Beep(1400,120); Beep(1400,120); }
    else         { Beep(900,200); }
}
