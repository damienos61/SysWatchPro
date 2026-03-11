#ifndef WIDGETS_H
#define WIDGETS_H

void widget_set_color(int color);
void widget_reset_color(void);
void widget_draw_bar(double percent, int width);
void widget_clear_screen(void);
void widget_set_title(const char* title);
void widget_print_header(const char* version, const char* mode);
void widget_print_section(const char* title, int color);
void widget_print_separator(void);
void widget_print_alert(const char* msg, int level); /* 1=warn 2=alert 3=crit */
void widget_print_footer(int procs, int suspects, int anomalies,
                          int netSuspects, const char* msg);
void widget_beep(int critical);

#endif
