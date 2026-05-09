#ifndef __UI_H
#define __UI_H

#include <stdint.h>

typedef enum {
    PAGE_HOME = 0,
    PAGE_DATA,
    PAGE_SETTING,
    PAGE_NET,
    PAGE_AI
} page_t;

extern page_t g_current_page;
extern uint8_t g_need_redraw;

void ui_draw_page(void);
void ui_refresh_current_page(void);
void ui_touch_process(uint16_t x, uint16_t y);

#endif
