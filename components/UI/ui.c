#include "ui.h"
#include "app_data.h"
#include "spilcd.h"
#include <stdio.h>

#define LCD_W   320
#define LCD_H   240

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
 * 根据实测四角坐标得到：
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
    spilcd_clear(WHITE);
}

static void ui_draw_home(void)
{
    ui_clear_page();

    spilcd_show_string(15, 10, 320, 24, 24, "Smart Env Monitor", RED);
    spilcd_show_string(15, 38, 320, 16, 16, "Main Node Home", BLUE);

    /* 系统状态区 */
    spilcd_show_string(15, 70, 320, 16, 16,
                       g_app.node_a_online ? "NodeA: ONLINE" : "NodeA: OFFLINE",
                       g_app.node_a_online ? BLACK : RED);

    spilcd_show_string(15, 95, 320, 16, 16,
                       g_app.node_b_online ? "NodeB: ONLINE" : "NodeB: OFFLINE",
                       g_app.node_b_online ? BLACK : RED);

    spilcd_show_string(15, 120, 320, 16, 16,
                       g_app.alarm_on ? "Alarm: ON" : "Alarm: OFF",
                       g_app.alarm_on ? RED : BLACK);

    spilcd_show_string(15, 145, 320, 16, 16,
                       g_app.wifi_ok ? "WiFi: OK" : "WiFi: OFF",
                       g_app.wifi_ok ? BLACK : RED);

    /* 四个功能入口 */
    spilcd_show_string(25, 185, 320, 24, 24, "[ DATA ]", BLACK);
    spilcd_show_string(180, 185, 320, 24, 24, "[ SET ]", BLACK);

    spilcd_show_string(25, 215, 320, 24, 24, "[ NET  ]", BLACK);
    spilcd_show_string(180, 215, 320, 24, 24, "[ AI  ]", BLACK);
}

static void ui_draw_data(void)
{
    char buf[64];

    ui_clear_page();

    spilcd_show_string(15, 10, 320, 24, 24, "NODE DATA PAGE", RED);

    /* NodeA */
    spilcd_show_string(15, 45, 320, 16, 16, "NodeA", BLUE);

    snprintf(buf, sizeof(buf), "Status: %s", g_app.node_a_online ? "ONLINE" : "OFFLINE");
    spilcd_show_string(15, 70, 320, 16, 16, buf, g_app.node_a_online ? BLACK : RED);

    snprintf(buf, sizeof(buf), "Temp  : %.1f C", g_app.temp);
    spilcd_show_string(15, 95, 320, 16, 16, buf, BLACK);

    snprintf(buf, sizeof(buf), "Humi  : %.1f %%", g_app.humi);
    spilcd_show_string(15, 120, 320, 16, 16, buf, BLACK);

    /* NodeB */
    spilcd_show_string(170, 45, 320, 16, 16, "NodeB", BLUE);

    snprintf(buf, sizeof(buf), "Status: %s", g_app.node_b_online ? "ONLINE" : "OFFLINE");
    spilcd_show_string(170, 70, 320, 16, 16, buf, g_app.node_b_online ? BLACK : RED);

    snprintf(buf, sizeof(buf), "Light : %.1f lx", g_app.light);
    spilcd_show_string(170, 95, 320, 16, 16, buf, BLACK);

    snprintf(buf, sizeof(buf), "Human : %s", g_app.human ? "YES" : "NO");
    spilcd_show_string(170, 120, 320, 16, 16, buf, BLACK);

    /* 底部提示 */
    spilcd_show_string(15, 175, 320, 16, 16, "This page shows node data", BLUE);

    /* BACK 按钮 */
    spilcd_show_string(20, 205, 320, 24, 24, "[ BACK ]", BLUE);
}

static void ui_draw_setting(void)
{
    char buf[64];

    ui_clear_page();

    spilcd_show_string(15, 10, 320, 24, 24, "SETTING PAGE", RED);

    /* 温度阈值 */
    snprintf(buf, sizeof(buf), "Temp Max : %.1f", g_app.temp_max);
    spilcd_show_string(15, 50, 320, 16, 16, buf, BLACK);
    spilcd_show_string(220, 50, 320, 16, 16, "[-] [+]", BLUE);

    /* 湿度阈值 */
    snprintf(buf, sizeof(buf), "Humi Max : %.1f", g_app.humi_max);
    spilcd_show_string(15, 85, 320, 16, 16, buf, BLACK);
    spilcd_show_string(220, 85, 320, 16, 16, "[-] [+]", BLUE);

    /* 光照阈值 */
    snprintf(buf, sizeof(buf), "Light Min: %.1f", g_app.light_min);
    spilcd_show_string(15, 120, 320, 16, 16, buf, BLACK);
    spilcd_show_string(220, 120, 320, 16, 16, "[-] [+]", BLUE);

    /* 蜂鸣器开关 */
    snprintf(buf, sizeof(buf), "Buzzer   : %s", g_app.buzzer_on ? "ON" : "OFF");
    spilcd_show_string(15, 165, 320, 16, 16, buf, BLACK);
    spilcd_show_string(180, 165, 320, 16, 16, "[ TOGGLE ]", BLUE);

    /* BACK */
    spilcd_show_string(20, 205, 320, 24, 24, "[ BACK ]", BLUE);
}

static void ui_draw_net(void)
{
    char buf[64];

    ui_clear_page();

    spilcd_show_string(15, 10, 320, 24, 24, "NET PAGE", RED);

    snprintf(buf, sizeof(buf), "WiFi : %s", g_app.wifi_ok ? "OK" : "OFF");
    spilcd_show_string(15, 60, 320, 16, 16, buf, g_app.wifi_ok ? BLACK : RED);

    snprintf(buf, sizeof(buf), "IP   : %s", g_app.wifi_ip);
    spilcd_show_string(15, 95, 320, 16, 16, buf, BLACK);

    snprintf(buf, sizeof(buf), "MQTT : %s", g_app.mqtt_ok ? "OK" : "OFF");
    spilcd_show_string(15, 130, 320, 16, 16, buf, g_app.mqtt_ok ? BLACK : RED);

    spilcd_show_string(15, 175, 320, 16, 16, "Network information page", BLUE);

    spilcd_show_string(20, 205, 320, 24, 24, "[ BACK ]", BLUE);
}


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
    

void ui_touch_process(uint16_t raw_x, uint16_t raw_y)
{
    uint16_t x, y;

    ui_map_touch_xy(raw_x, raw_y, &x, &y);

    switch (g_current_page) {
        case PAGE_HOME:
  
    /* DATA */
        if (ui_in_rect(x, y, 20, 175, 140, 205)) {
            g_current_page = PAGE_DATA;
            g_need_redraw = 1;
        }
    /* SET */
        else if (ui_in_rect(x, y, 170, 175, 310, 205)) {
            g_current_page = PAGE_SETTING;
            g_need_redraw = 1;
        }
    /* NET：先不切页，后面再做 */
        else if (ui_in_rect(x, y, 20, 205, 140, 239)) {
             g_current_page = PAGE_NET;
             g_need_redraw = 1;
        }
    /* AI：先不切页，后面再做 */
        else if (ui_in_rect(x, y, 170, 205, 310, 239)) {
        /* 暂时先不处理 */
        }
        break;

            

        case PAGE_DATA:
            /* BACK */
            if (ui_in_rect(x, y, 10, 190, 130, 239)) {
                g_current_page = PAGE_HOME;
                g_need_redraw = 1;
            }
            break;

         case PAGE_SETTING:
    /* Temp Max: [-] */
            if (ui_in_rect(x, y, 220, 40, 255, 70)) {
                g_app.temp_max -= 1.0f;
                g_need_redraw = 1;
            }
    /* Temp Max: [+] */
            else if (ui_in_rect(x, y, 260, 40, 310, 70)) {
                g_app.temp_max += 1.0f;
                g_need_redraw = 1;
            }

    /* Humi Max: [-] */
            else if (ui_in_rect(x, y, 220, 75, 255, 105)) {
                g_app.humi_max -= 1.0f;
                g_need_redraw = 1;
            }
    /* Humi Max: [+] */
            else if (ui_in_rect(x, y, 260, 75, 310, 105)) {
                g_app.humi_max += 1.0f;
                g_need_redraw = 1;
            }

    /* Light Min: [-] */
            else if (ui_in_rect(x, y, 220, 110, 255, 140)) {
                g_app.light_min -= 1.0f;
                g_need_redraw = 1;
            }
    /* Light Min: [+] */
            else if (ui_in_rect(x, y, 260, 110, 310, 140)) {
                g_app.light_min += 1.0f;
                g_need_redraw = 1;
            }

    /* TOGGLE BUZZER */
            else if (ui_in_rect(x, y, 170, 150, 310, 185)) {
                g_app.buzzer_on = !g_app.buzzer_on;
                g_need_redraw = 1;
            }

    /* BACK */
            else if (ui_in_rect(x, y, 10, 190, 130, 239)) {
                g_current_page = PAGE_HOME;
                g_need_redraw = 1;
            }
            break;

            case PAGE_NET:
    /* BACK */
            if (ui_in_rect(x, y, 10, 190, 130, 239)) {
                g_current_page = PAGE_HOME;
                g_need_redraw = 1;
            }
            break;

        default:
            break;
    }
}