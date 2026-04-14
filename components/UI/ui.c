#include "ui.h"
#include "app_data.h"
#include "spilcd.h"
#include <stdio.h>
#include <string.h>

#define LCD_W   320
#define LCD_H   240

#define UI_BG_COLOR      WHITE
#define UI_TEXT_COLOR    BLACK
#define UI_WARN_COLOR    RED
#define UI_TITLE_COLOR   RED
#define UI_LABEL_COLOR   BLUE

page_t g_current_page = PAGE_HOME;
uint8_t g_need_redraw = 1;

/* 判断点是否在矩形区域内 */
static uint8_t ui_in_rect(uint16_t x, uint16_t y,
                          uint16_t x1, uint16_t y1,
                          uint16_t x2, uint16_t y2)
{
    return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
}

/*
 * 触摸坐标校准：
 * screen_x = raw_y
 * screen_y = 239 - raw_x
 */
static void ui_map_touch_xy(uint16_t raw_x, uint16_t raw_y,
                            uint16_t *screen_x, uint16_t *screen_y)
{
    *screen_x = raw_y;
    *screen_y = 239 - raw_x;
}

static void ui_clear_page(void)
{
    spilcd_clear(UI_BG_COLOR);
}

/* 清一行文本区域，再重写 */
static void ui_clear_line(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    spilcd_fill(x, y, x + w - 1, y + h - 1, UI_BG_COLOR);
}

static void ui_draw_text_line(uint16_t x, uint16_t y,
                              uint16_t area_w, uint16_t font_size,
                              const char *text, uint16_t color)
{
    spilcd_show_string(x, y, area_w, font_size, font_size, (char *)text, color);
}

/* ========================= HOME PAGE ========================= */

static void ui_draw_home_static(void)
{
    ui_clear_page();

    spilcd_show_string(15, 10, 320, 24, 24, "Smart Env Monitor", UI_TITLE_COLOR);
    spilcd_show_string(15, 38, 320, 16, 16, "Main Node Home", UI_LABEL_COLOR);

    /* 四个功能入口 */
    spilcd_show_string(25, 185, 320, 24, 24, "[ DATA ]", UI_TEXT_COLOR);
    spilcd_show_string(180, 185, 320, 24, 24, "[ SET ]", UI_TEXT_COLOR);
    spilcd_show_string(25, 215, 320, 24, 24, "[ NET  ]", UI_TEXT_COLOR);
    spilcd_show_string(180, 215, 320, 24, 24, "[ AI  ]", UI_TEXT_COLOR);
}

static void ui_refresh_home_dynamic(void)
{
    ui_clear_line(15, 70, 220, 18);
    ui_draw_text_line(15, 70, 220, 16,
                      g_app.node_a_online ? "NodeA: ONLINE" : "NodeA: OFFLINE",
                      g_app.node_a_online ? UI_TEXT_COLOR : UI_WARN_COLOR);

    ui_clear_line(15, 95, 220, 18);
    ui_draw_text_line(15, 95, 220, 16,
                      g_app.node_b_online ? "NodeB: ONLINE" : "NodeB: OFFLINE",
                      g_app.node_b_online ? UI_TEXT_COLOR : UI_WARN_COLOR);

    ui_clear_line(15, 120, 220, 18);
    ui_draw_text_line(15, 120, 220, 16,
                      g_app.alarm_on ? "Alarm: ON" : "Alarm: OFF",
                      g_app.alarm_on ? UI_WARN_COLOR : UI_TEXT_COLOR);

    ui_clear_line(15, 145, 220, 18);
    ui_draw_text_line(15, 145, 220, 16,
                      g_app.wifi_ok ? "WiFi: OK" : "WiFi: OFF",
                      g_app.wifi_ok ? UI_TEXT_COLOR : UI_WARN_COLOR);
}

static void ui_draw_home(void)
{
    ui_draw_home_static();
    ui_refresh_home_dynamic();
}

/* ========================= DATA PAGE ========================= */

static void ui_draw_data_static(void)
{
    ui_clear_page();

    spilcd_show_string(15, 10, 320, 24, 24, "NODE DATA PAGE", UI_TITLE_COLOR);

    spilcd_show_string(15, 45, 320, 16, 16, "NodeA", UI_LABEL_COLOR);
    spilcd_show_string(170, 45, 320, 16, 16, "NodeB", UI_LABEL_COLOR);

    spilcd_show_string(15, 175, 320, 16, 16, "This page shows node data", UI_LABEL_COLOR);
    spilcd_show_string(20, 205, 320, 24, 24, "[ BACK ]", UI_LABEL_COLOR);
}

static void ui_refresh_data_dynamic(void)
{
    char buf[64];

    ui_clear_line(15, 70, 140, 18);
    snprintf(buf, sizeof(buf), "Status: %s", g_app.node_a_online ? "ONLINE" : "OFFLINE");
    ui_draw_text_line(15, 70, 140, 16, buf, g_app.node_a_online ? UI_TEXT_COLOR : UI_WARN_COLOR);

    ui_clear_line(15, 95, 140, 18);
    snprintf(buf, sizeof(buf), "Temp  : %.1f C", g_app.temp);
    ui_draw_text_line(15, 95, 140, 16, buf, UI_TEXT_COLOR);

    ui_clear_line(15, 120, 140, 18);
    snprintf(buf, sizeof(buf), "Humi  : %.1f %%", g_app.humi);
    ui_draw_text_line(15, 120, 140, 16, buf, UI_TEXT_COLOR);

    ui_clear_line(170, 70, 140, 18);
    snprintf(buf, sizeof(buf), "Status: %s", g_app.node_b_online ? "ONLINE" : "OFFLINE");
    ui_draw_text_line(170, 70, 140, 16, buf, g_app.node_b_online ? UI_TEXT_COLOR : UI_WARN_COLOR);

    ui_clear_line(170, 95, 140, 18);
    snprintf(buf, sizeof(buf), "Light : %.1f lx", g_app.light);
    ui_draw_text_line(170, 95, 140, 16, buf, UI_TEXT_COLOR);

    ui_clear_line(170, 120, 140, 18);
    snprintf(buf, sizeof(buf), "Human : %s", g_app.human ? "YES" : "NO");
    ui_draw_text_line(170, 120, 140, 16, buf, UI_TEXT_COLOR);
}

static void ui_draw_data(void)
{
    ui_draw_data_static();
    ui_refresh_data_dynamic();
}

/* ========================= SETTING PAGE ========================= */

static void ui_draw_setting_static(void)
{
    ui_clear_page();

    spilcd_show_string(15, 10, 320, 24, 24, "SETTING PAGE", UI_TITLE_COLOR);

    spilcd_show_string(220, 50, 320, 16, 16, "[-] [+]", UI_LABEL_COLOR);
    spilcd_show_string(220, 85, 320, 16, 16, "[-] [+]", UI_LABEL_COLOR);
    spilcd_show_string(220, 120, 320, 16, 16, "[-] [+]", UI_LABEL_COLOR);
    spilcd_show_string(180, 165, 320, 16, 16, "[ TOGGLE ]", UI_LABEL_COLOR);

    spilcd_show_string(20, 205, 320, 24, 24, "[ BACK ]", UI_LABEL_COLOR);
}

static void ui_refresh_setting_dynamic(void)
{
    char buf[64];

    ui_clear_line(15, 50, 180, 18);
    snprintf(buf, sizeof(buf), "Temp Max : %.1f", g_app.temp_max);
    ui_draw_text_line(15, 50, 180, 16, buf, UI_TEXT_COLOR);

    ui_clear_line(15, 85, 180, 18);
    snprintf(buf, sizeof(buf), "Humi Max : %.1f", g_app.humi_max);
    ui_draw_text_line(15, 85, 180, 16, buf, UI_TEXT_COLOR);

    ui_clear_line(15, 120, 180, 18);
    snprintf(buf, sizeof(buf), "Light Min: %.1f", g_app.light_min);
    ui_draw_text_line(15, 120, 180, 16, buf, UI_TEXT_COLOR);

    ui_clear_line(15, 165, 150, 18);
    snprintf(buf, sizeof(buf), "Buzzer   : %s", g_app.buzzer_on ? "ON" : "OFF");
    ui_draw_text_line(15, 165, 150, 16, buf, UI_TEXT_COLOR);
}

static void ui_draw_setting(void)
{
    ui_draw_setting_static();
    ui_refresh_setting_dynamic();
}

/* ========================= NET PAGE ========================= */

static void ui_draw_net_static(void)
{
    ui_clear_page();

    spilcd_show_string(15, 10, 320, 24, 24, "NET PAGE", UI_TITLE_COLOR);
    spilcd_show_string(15, 175, 320, 16, 16, "Network information page", UI_LABEL_COLOR);
    spilcd_show_string(20, 205, 320, 24, 24, "[ BACK ]", UI_LABEL_COLOR);
}

static void ui_refresh_net_dynamic(void)
{
    char buf[64];

    ui_clear_line(15, 60, 220, 18);
    snprintf(buf, sizeof(buf), "WiFi : %s", g_app.wifi_ok ? "OK" : "OFF");
    ui_draw_text_line(15, 60, 220, 16, buf, g_app.wifi_ok ? UI_TEXT_COLOR : UI_WARN_COLOR);

    ui_clear_line(15, 95, 220, 18);
    snprintf(buf, sizeof(buf), "IP   : %s", g_app.wifi_ip);
    ui_draw_text_line(15, 95, 220, 16, buf, UI_TEXT_COLOR);

    ui_clear_line(15, 130, 220, 18);
    snprintf(buf, sizeof(buf), "MQTT : %s", g_app.mqtt_ok ? "OK" : "OFF");
    ui_draw_text_line(15, 130, 220, 16, buf, g_app.mqtt_ok ? UI_TEXT_COLOR : UI_WARN_COLOR);
}

static void ui_draw_net(void)
{
    ui_draw_net_static();
    ui_refresh_net_dynamic();
}

/* ========================= PUBLIC ========================= */

void ui_draw_page(void)
{
    switch (g_current_page) {
        case PAGE_HOME:
            ui_draw_home();
            break;

        case PAGE_DATA:
            ui_draw_data();
            break;

        case PAGE_SETTING:
            ui_draw_setting();
            break;

        case PAGE_NET:
            ui_draw_net();
            break;

        default:
            ui_draw_home();
            break;
    }

    g_need_redraw = 0;
}

void ui_refresh_current_page(void)
{
    switch (g_current_page) {
        case PAGE_HOME:
            ui_refresh_home_dynamic();
            break;

        case PAGE_DATA:
            ui_refresh_data_dynamic();
            break;

        case PAGE_SETTING:
            ui_refresh_setting_dynamic();
            break;

        case PAGE_NET:
            ui_refresh_net_dynamic();
            break;

        default:
            break;
    }

    g_need_redraw = 0;
}

void ui_touch_process(uint16_t raw_x, uint16_t raw_y)
{
    uint16_t x, y;

    ui_map_touch_xy(raw_x, raw_y, &x, &y);

    switch (g_current_page) {
        case PAGE_HOME:
            if (ui_in_rect(x, y, 20, 175, 140, 205)) {
                g_current_page = PAGE_DATA;
                g_need_redraw = 1;
            }
            else if (ui_in_rect(x, y, 170, 175, 310, 205)) {
                g_current_page = PAGE_SETTING;
                g_need_redraw = 1;
            }
            else if (ui_in_rect(x, y, 20, 205, 140, 239)) {
                g_current_page = PAGE_NET;
                g_need_redraw = 1;
            }
            else if (ui_in_rect(x, y, 170, 205, 310, 239)) {
                /* 暂时先不处理 */
            }
            break;

        case PAGE_DATA:
            if (ui_in_rect(x, y, 10, 190, 130, 239)) {
                g_current_page = PAGE_HOME;
                g_need_redraw = 1;
            }
            break;

        case PAGE_SETTING:
            if (ui_in_rect(x, y, 220, 40, 255, 70)) {
                g_app.temp_max -= 1.0f;
                ui_refresh_current_page();
            }
            else if (ui_in_rect(x, y, 260, 40, 310, 70)) {
                g_app.temp_max += 1.0f;
                ui_refresh_current_page();
            }
            else if (ui_in_rect(x, y, 220, 75, 255, 105)) {
                g_app.humi_max -= 1.0f;
                ui_refresh_current_page();
            }
            else if (ui_in_rect(x, y, 260, 75, 310, 105)) {
                g_app.humi_max += 1.0f;
                ui_refresh_current_page();
            }
            else if (ui_in_rect(x, y, 220, 110, 255, 140)) {
                g_app.light_min -= 1.0f;
                ui_refresh_current_page();
            }
            else if (ui_in_rect(x, y, 260, 110, 310, 140)) {
                g_app.light_min += 1.0f;
                ui_refresh_current_page();
            }
            else if (ui_in_rect(x, y, 170, 150, 310, 185)) {
                g_app.buzzer_on = !g_app.buzzer_on;
                ui_refresh_current_page();
            }
            else if (ui_in_rect(x, y, 10, 190, 130, 239)) {
                g_current_page = PAGE_HOME;
                g_need_redraw = 1;
            }
            break;

        case PAGE_NET:
            if (ui_in_rect(x, y, 10, 190, 130, 239)) {
                g_current_page = PAGE_HOME;
                g_need_redraw = 1;
            }
            break;

        default:
            break;
    }
}