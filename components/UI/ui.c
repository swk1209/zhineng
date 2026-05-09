#include "ui.h"
#include "app_data.h"
#include "app_service.h"
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

#define UI_FONT_16       16
#define UI_FONT_24       24

/* 16号字体单字符宽度 = 8 */
#define UI_CHAR_W_16     8
#define UI_CHAR_H_16     16

page_t g_current_page = PAGE_HOME;
uint8_t g_need_redraw = 1;

/* 页面缓存，用来判断“谁变了才刷谁” */
static app_data_t s_ui_last_app;
static uint8_t s_ui_cache_valid = 0;

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

static void ui_clear_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    spilcd_fill(x, y, x + w - 1, y + h - 1, UI_BG_COLOR);
}

static void ui_draw_text(uint16_t x, uint16_t y,
                         uint16_t area_w, uint16_t font_size,
                         const char *text, uint16_t color)
{
    spilcd_show_string(x, y, area_w, font_size, font_size, (char *)text, color);
}

/* 只刷新一个值区，标签不动 */
static void ui_draw_value_field(uint16_t x, uint16_t y,
                                uint8_t max_chars,
                                const char *text,
                                uint16_t color)
{
    uint16_t w = max_chars * UI_CHAR_W_16;
    ui_clear_rect(x, y, w, UI_CHAR_H_16);
    ui_draw_text(x, y, w, UI_FONT_16, text, color);
}

static void ui_sync_cache_from_app(void)
{
    memcpy(&s_ui_last_app, &g_app, sizeof(app_data_t));
    s_ui_cache_valid = 1;
}

static void ui_save_settings(void)
{
    app_service_save_settings();
}

/* ========================= HOME PAGE ========================= */

static void ui_draw_home_static(void)
{
    ui_clear_page();

    spilcd_show_string(15, 10, 320, 24, 24, "Smart Env Monitor", UI_TITLE_COLOR);
    spilcd_show_string(15, 38, 320, 16, 16, "Main Node Home", UI_LABEL_COLOR);

    /* 固定标签 */
    spilcd_show_string(15, 70, 320, 16, 16, "NodeA:", UI_TEXT_COLOR);
    spilcd_show_string(15, 95, 320, 16, 16, "NodeB:", UI_TEXT_COLOR);
    spilcd_show_string(15, 120, 320, 16, 16, "Alarm:", UI_TEXT_COLOR);
    spilcd_show_string(15, 145, 320, 16, 16, "WiFi :", UI_TEXT_COLOR);

    /* 四个功能入口 */
    spilcd_show_string(25, 185, 320, 24, 24, "[ DATA ]", UI_TEXT_COLOR);
    spilcd_show_string(180, 185, 320, 24, 24, "[ SET ]", UI_TEXT_COLOR);
    spilcd_show_string(25, 215, 320, 24, 24, "[ NET  ]", UI_TEXT_COLOR);
    spilcd_show_string(180, 215, 320, 24, 24, "[ AI  ]", UI_TEXT_COLOR);
}

static void ui_refresh_home_node_a(void)
{
    ui_draw_value_field(70, 70, 8,
                        g_app.node_a_online ? "ONLINE" : "OFFLINE",
                        g_app.node_a_online ? UI_TEXT_COLOR : UI_WARN_COLOR);
}

static void ui_refresh_home_node_b(void)
{
    ui_draw_value_field(70, 95, 8,
                        g_app.node_b_online ? "ONLINE" : "OFFLINE",
                        g_app.node_b_online ? UI_TEXT_COLOR : UI_WARN_COLOR);
}

static void ui_refresh_home_alarm(void)
{
    ui_draw_value_field(70, 120, 4,
                        g_app.alarm_on ? "ON" : "OFF",
                        g_app.alarm_on ? UI_WARN_COLOR : UI_TEXT_COLOR);
}

static void ui_refresh_home_wifi(void)
{
    ui_draw_value_field(70, 145, 4,
                        g_app.wifi_ok ? "OK" : "OFF",
                        g_app.wifi_ok ? UI_TEXT_COLOR : UI_WARN_COLOR);
}

static void ui_refresh_home_dynamic_all(void)
{
    ui_refresh_home_node_a();
    ui_refresh_home_node_b();
    ui_refresh_home_alarm();
    ui_refresh_home_wifi();
}

static void ui_refresh_home_dynamic_changed(void)
{
    if (!s_ui_cache_valid) {
        ui_refresh_home_dynamic_all();
        return;
    }

    if (s_ui_last_app.node_a_online != g_app.node_a_online) {
        ui_refresh_home_node_a();
    }

    if (s_ui_last_app.node_b_online != g_app.node_b_online) {
        ui_refresh_home_node_b();
    }

    if (s_ui_last_app.alarm_on != g_app.alarm_on) {
        ui_refresh_home_alarm();
    }

    if (s_ui_last_app.wifi_ok != g_app.wifi_ok) {
        ui_refresh_home_wifi();
    }
}

static void ui_draw_home(void)
{
    ui_draw_home_static();
    ui_refresh_home_dynamic_all();
}

/* ========================= DATA PAGE ========================= */

static void ui_draw_data_static(void)
{
    ui_clear_page();

    spilcd_show_string(15, 10, 320, 24, 24, "NODE DATA PAGE", UI_TITLE_COLOR);

    spilcd_show_string(15, 45, 320, 16, 16, "NodeA", UI_LABEL_COLOR);
    spilcd_show_string(170, 45, 320, 16, 16, "NodeB", UI_LABEL_COLOR);

    /* NodeA 固定标签 */
    spilcd_show_string(15, 70, 320, 16, 16, "Status:", UI_TEXT_COLOR);
    spilcd_show_string(15, 95, 320, 16, 16, "Temp  :", UI_TEXT_COLOR);
    spilcd_show_string(15, 120, 320, 16, 16, "Humi  :", UI_TEXT_COLOR);
    spilcd_show_string(120, 95, 320, 16, 16, "C", UI_TEXT_COLOR);
    spilcd_show_string(120, 120, 320, 16, 16, "%", UI_TEXT_COLOR);

    /* NodeB 固定标签 */
    spilcd_show_string(170, 70, 320, 16, 16, "Status:", UI_TEXT_COLOR);
    spilcd_show_string(170, 95, 320, 16, 16, "Light :", UI_TEXT_COLOR);
    spilcd_show_string(170, 120, 320, 16, 16, "Human :", UI_TEXT_COLOR);
    spilcd_show_string(275, 95, 320, 16, 16, "lx", UI_TEXT_COLOR);

    spilcd_show_string(15, 175, 320, 16, 16, "This page shows node data", UI_LABEL_COLOR);
    spilcd_show_string(20, 205, 320, 24, 24, "[ BACK ]", UI_LABEL_COLOR);
}

static void ui_refresh_data_node_a_status(void)
{
    ui_draw_value_field(75, 70, 8,
                        g_app.node_a_online ? "ONLINE" : "OFFLINE",
                        g_app.node_a_online ? UI_TEXT_COLOR : UI_WARN_COLOR);
}

static void ui_refresh_data_temp(void)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%5.1f", g_app.temp);
    ui_draw_value_field(70, 95, 6, buf, UI_TEXT_COLOR);
}

static void ui_refresh_data_humi(void)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%5.1f", g_app.humi);
    ui_draw_value_field(70, 120, 6, buf, UI_TEXT_COLOR);
}

static void ui_refresh_data_node_b_status(void)
{
    ui_draw_value_field(230, 70, 8,
                        g_app.node_b_online ? "ONLINE" : "OFFLINE",
                        g_app.node_b_online ? UI_TEXT_COLOR : UI_WARN_COLOR);
}

static void ui_refresh_data_light(void)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%5.1f", g_app.light);
    ui_draw_value_field(225, 95, 6, buf, UI_TEXT_COLOR);
}

static void ui_refresh_data_human(void)
{
    ui_draw_value_field(225, 120, 4,
                        g_app.human ? "YES" : "NO",
                        UI_TEXT_COLOR);
}

static void ui_refresh_data_dynamic_all(void)
{
    ui_refresh_data_node_a_status();
    ui_refresh_data_temp();
    ui_refresh_data_humi();
    ui_refresh_data_node_b_status();
    ui_refresh_data_light();
    ui_refresh_data_human();
}

static void ui_draw_data(void)
{
    ui_draw_data_static();
    ui_refresh_data_dynamic_all();
}

/* ========================= SETTING PAGE ========================= */

static void ui_draw_setting_static(void)
{
    ui_clear_page();

    spilcd_show_string(15, 10, 320, 24, 24, "SETTING PAGE", UI_TITLE_COLOR);

    /* 固定标签 */
    spilcd_show_string(15, 50, 320, 16, 16, "Temp Max :", UI_TEXT_COLOR);
    spilcd_show_string(15, 85, 320, 16, 16, "Humi Max :", UI_TEXT_COLOR);
    spilcd_show_string(15, 120, 320, 16, 16, "Light Min:", UI_TEXT_COLOR);
    ui_clear_rect(15, 165, 90, UI_CHAR_H_16);
    spilcd_show_string(15, 165, 320, 16, 16, "Buzzer   :", UI_TEXT_COLOR);

    spilcd_show_string(220, 50, 320, 16, 16, "[-] [+]", UI_LABEL_COLOR);
    spilcd_show_string(220, 85, 320, 16, 16, "[-] [+]", UI_LABEL_COLOR);
    spilcd_show_string(220, 120, 320, 16, 16, "[-] [+]", UI_LABEL_COLOR);
    spilcd_show_string(180, 165, 320, 16, 16, "[ TOGGLE ]", UI_LABEL_COLOR);

    spilcd_show_string(20, 205, 320, 24, 24, "[ BACK ]", UI_LABEL_COLOR);
}

static void ui_refresh_setting_temp(void)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%5.1f", g_app.temp_max);
    ui_draw_value_field(100, 50, 6, buf, UI_TEXT_COLOR);
}

static void ui_refresh_setting_humi(void)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%5.1f", g_app.humi_max);
    ui_draw_value_field(100, 85, 6, buf, UI_TEXT_COLOR);
}

static void ui_refresh_setting_light(void)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%5.1f", g_app.light_min);
    ui_draw_value_field(100, 120, 6, buf, UI_TEXT_COLOR);
}

static void ui_refresh_setting_buzzer(void)
{
    ui_draw_value_field(100, 165, 4,
                        g_app.buzzer_on ? "ON" : "OFF",
                        UI_TEXT_COLOR);
}

static void ui_refresh_setting_dynamic_all(void)
{
    ui_refresh_setting_temp();
    ui_refresh_setting_humi();
    ui_refresh_setting_light();
    ui_refresh_setting_buzzer();
}

static void ui_draw_setting(void)
{
    ui_draw_setting_static();
    ui_refresh_setting_dynamic_all();
}

/* ========================= NET PAGE ========================= */

static void ui_draw_net_static(void)
{
    ui_clear_page();

    spilcd_show_string(15, 10, 320, 24, 24, "NET PAGE", UI_TITLE_COLOR);

    spilcd_show_string(15, 60, 320, 16, 16, "WiFi :", UI_TEXT_COLOR);
    spilcd_show_string(15, 95, 320, 16, 16, "IP   :", UI_TEXT_COLOR);
    spilcd_show_string(15, 130, 320, 16, 16, "CH   :", UI_TEXT_COLOR);
    spilcd_show_string(15, 155, 320, 16, 16, "MAC  :", UI_TEXT_COLOR);

    spilcd_show_string(20, 205, 320, 24, 24, "[ BACK ]", UI_LABEL_COLOR);
}

static void ui_refresh_net_wifi(void)
{
    ui_draw_value_field(70, 60, 4,
                        g_app.wifi_ok ? "OK" : "OFF",
                        g_app.wifi_ok ? UI_TEXT_COLOR : UI_WARN_COLOR);
}

static void ui_refresh_net_ip(void)
{
    ui_draw_value_field(70, 95, 16, g_app.wifi_ip, UI_TEXT_COLOR);
}

static void ui_refresh_net_channel(void)
{
    char buf[8];
    snprintf(buf, sizeof(buf), "%u", g_app.wifi_channel);
    ui_draw_value_field(70, 130, 3, buf, UI_TEXT_COLOR);
}

static void ui_refresh_net_mac(void)
{
    ui_draw_value_field(70, 155, 18, g_app.wifi_mac, UI_TEXT_COLOR);
}

static void ui_refresh_net_dynamic_all(void)
{
    ui_refresh_net_wifi();
    ui_refresh_net_ip();
    ui_refresh_net_channel();
    ui_refresh_net_mac();
}

static void ui_refresh_net_dynamic_changed(void)
{
    if (!s_ui_cache_valid) {
        ui_refresh_net_dynamic_all();
        return;
    }

    if (s_ui_last_app.wifi_ok != g_app.wifi_ok) {
        ui_refresh_net_wifi();
    }

    if (strncmp(s_ui_last_app.wifi_ip, g_app.wifi_ip, sizeof(g_app.wifi_ip)) != 0) {
        ui_refresh_net_ip();
    }

    if (s_ui_last_app.wifi_channel != g_app.wifi_channel) {
        ui_refresh_net_channel();
    }

    if (strncmp(s_ui_last_app.wifi_mac, g_app.wifi_mac, sizeof(g_app.wifi_mac)) != 0) {
        ui_refresh_net_mac();
    }
}

static void ui_draw_net(void)
{
    ui_draw_net_static();
    ui_refresh_net_dynamic_all();
}

/* ========================= AI PAGE ========================= */

static uint8_t ui_ai_has_node_alarm(void)
{
    if (g_app.node_a_online &&
        (g_app.temp > g_app.temp_max ||
         g_app.humi > g_app.humi_max ||
         g_app.light < g_app.light_min)) {
        return 1;
    }

    if (g_app.node_b_online && g_app.human) {
        return 1;
    }

    return 0;
}

static const char *ui_ai_get_advice_line1(void)
{
    if (!g_app.node_a_online || !g_app.node_b_online) {
        return "Check offline node";
    }

    if (g_app.alarm_on) {
        return "Handle alarm source";
    }

    if (!g_app.wifi_ok) {
        return "WiFi offline, local mode";
    }

    return "Keep current settings";
}

static const char *ui_ai_get_advice_line2(void)
{
    if (!g_app.node_a_online) {
        return "NodeA link/power needed";
    }

    if (!g_app.node_b_online) {
        return "NodeB link/power needed";
    }

    if (g_app.node_a_online && g_app.temp > g_app.temp_max) {
        return "Cool down the area";
    }

    if (g_app.node_a_online && g_app.humi > g_app.humi_max) {
        return "Reduce humidity";
    }

    if (g_app.node_a_online && g_app.light < g_app.light_min) {
        return "Increase light level";
    }

    if (g_app.node_b_online && g_app.human) {
        return "Confirm human activity";
    }

    if (!g_app.wifi_ok) {
        return "Cloud upload paused";
    }

    return "No action required";
}

static const char *ui_ai_get_analysis_line1(void)
{
    if (!g_app.node_a_online && !g_app.node_b_online) {
        return "Both nodes offline";
    }

    if (!g_app.node_a_online) {
        return "NodeA data missing";
    }

    if (!g_app.node_b_online) {
        return "NodeB data missing";
    }

    if (ui_ai_has_node_alarm()) {
        return "Anomaly detected";
    }

    return "No anomaly detected";
}

static const char *ui_ai_get_analysis_line2(void)
{
    if (g_app.node_a_online && g_app.temp > g_app.temp_max) {
        return "Temp above max limit";
    }

    if (g_app.node_a_online && g_app.humi > g_app.humi_max) {
        return "Humi above max limit";
    }

    if (g_app.node_a_online && g_app.light < g_app.light_min) {
        return "Light below min limit";
    }

    if (g_app.node_b_online && g_app.human) {
        return "Human presence active";
    }

    if (!g_app.wifi_ok) {
        return "Network needs attention";
    }

    return "Values within limits";
}

static void ui_draw_ai_static(void)
{
    ui_clear_page();

    spilcd_show_string(15, 10, 320, 24, 24, "AI ASSIST PAGE", UI_TITLE_COLOR);

    spilcd_show_string(15, 45, 320, 16, 16, "Smart Advice", UI_LABEL_COLOR);
    spilcd_show_string(15, 105, 320, 16, 16, "Anomaly Analysis", UI_LABEL_COLOR);

    spilcd_show_string(20, 205, 320, 24, 24, "[ BACK ]", UI_LABEL_COLOR);
}

static void ui_refresh_ai_advice(void)
{
    ui_draw_value_field(15, 68, 36, ui_ai_get_advice_line1(),
                        g_app.alarm_on ? UI_WARN_COLOR : UI_TEXT_COLOR);
    ui_draw_value_field(15, 86, 36, ui_ai_get_advice_line2(),
                        g_app.alarm_on ? UI_WARN_COLOR : UI_TEXT_COLOR);
}

static void ui_refresh_ai_analysis(void)
{
    ui_draw_value_field(15, 128, 36, ui_ai_get_analysis_line1(),
                        ui_ai_has_node_alarm() ? UI_WARN_COLOR : UI_TEXT_COLOR);
    ui_draw_value_field(15, 146, 36, ui_ai_get_analysis_line2(),
                        ui_ai_has_node_alarm() ? UI_WARN_COLOR : UI_TEXT_COLOR);
}

static void ui_refresh_ai_dynamic_all(void)
{
    ui_refresh_ai_advice();
    ui_refresh_ai_analysis();
}

static void ui_draw_ai(void)
{
    ui_draw_ai_static();
    ui_refresh_ai_dynamic_all();
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

        case PAGE_AI:
            ui_draw_ai();
            break;

        default:
            ui_draw_home();
            break;
    }

    ui_sync_cache_from_app();
    g_need_redraw = 0;
}

void ui_refresh_current_page(void)
{
    switch (g_current_page) {
        case PAGE_HOME:
            ui_refresh_home_dynamic_changed();
            break;

        case PAGE_DATA:
            /* DATA页保持成组刷新 */
            ui_refresh_data_dynamic_all();
            break;

        case PAGE_SETTING:
            /* 设置页不做实时刷新，避免被后台状态打扰 */
            break;

        case PAGE_NET:
            ui_refresh_net_dynamic_changed();
            break;

        case PAGE_AI:
            ui_refresh_ai_dynamic_all();
            break;
        

        default:
            break;
    }

    ui_sync_cache_from_app();
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
                g_current_page = PAGE_AI;
                g_need_redraw = 1;
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
                ui_refresh_setting_temp();
                ui_save_settings();
                ui_sync_cache_from_app();
            }
            else if (ui_in_rect(x, y, 260, 40, 310, 70)) {
                g_app.temp_max += 1.0f;
                ui_refresh_setting_temp();
                ui_save_settings();
                ui_sync_cache_from_app();
            }
            else if (ui_in_rect(x, y, 220, 75, 255, 105)) {
                g_app.humi_max -= 1.0f;
                ui_refresh_setting_humi();
                ui_save_settings();
                ui_sync_cache_from_app();
            }
            else if (ui_in_rect(x, y, 260, 75, 310, 105)) {
                g_app.humi_max += 1.0f;
                ui_refresh_setting_humi();
                ui_save_settings();
                ui_sync_cache_from_app();
            }
            else if (ui_in_rect(x, y, 220, 110, 255, 140)) {
                g_app.light_min -= 1.0f;
                ui_refresh_setting_light();
                ui_save_settings();
                ui_sync_cache_from_app();
            }
            else if (ui_in_rect(x, y, 260, 110, 310, 140)) {
                g_app.light_min += 1.0f;
                ui_refresh_setting_light();
                ui_save_settings();
                ui_sync_cache_from_app();
            }
            else if (ui_in_rect(x, y, 170, 150, 310, 185)) {
                g_app.buzzer_on = !g_app.buzzer_on;
                app_service_apply_alarm_output();
                ui_refresh_setting_buzzer();
                ui_save_settings();
                ui_sync_cache_from_app();
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

        case PAGE_AI:
            if (ui_in_rect(x, y, 10, 190, 130, 239)) {
                g_current_page = PAGE_HOME;
                g_need_redraw = 1;
            }
            break;

        default:
            break;
    }
}
