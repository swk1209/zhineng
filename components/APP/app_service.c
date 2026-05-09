#include "app_service.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "esp_log.h"

#include "app_data.h"
#include "wifi_service.h"
#include "espnow_service.h"
#include "buzzer.h"

/* 节点离线超时时间 */
#define NODE_OFFLINE_TIMEOUT_MS   10000
#define APP_SETTINGS_MAGIC        0x41504647u

static TickType_t s_node_a_last_rx_tick = 0;
static TickType_t s_node_b_last_rx_tick = 0;
static const char *TAG = "APP_SERVICE";

typedef struct {
    uint32_t magic;
    float temp_max;
    float humi_max;
    float light_min;
    uint8_t buzzer_on;
} app_settings_nvs_t;

static esp_err_t app_load_settings(void)
{
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open("app_cfg", NVS_READONLY, &nvs);

    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        return ESP_OK;
    }

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "open app_cfg failed: %s", esp_err_to_name(ret));
        return ret;
    }

    app_settings_nvs_t settings;
    size_t len = sizeof(settings);

    ret = nvs_get_blob(nvs, "settings", &settings, &len);
    nvs_close(nvs);

    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        return ESP_OK;
    }

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "read settings failed: %s", esp_err_to_name(ret));
        return ret;
    }

    if (len != sizeof(settings) || settings.magic != APP_SETTINGS_MAGIC) {
        ESP_LOGW(TAG, "ignore invalid settings");
        return ESP_OK;
    }

    g_app.temp_max = settings.temp_max;
    g_app.humi_max = settings.humi_max;
    g_app.light_min = settings.light_min;
    g_app.buzzer_on = settings.buzzer_on ? 1 : 0;

    return ESP_OK;
}

static bool app_evaluate_alarm(void)
{
    bool alarm = false;

    if (g_app.node_a_online) {
        if (g_app.temp > g_app.temp_max ||
            g_app.humi > g_app.humi_max ||
            g_app.light < g_app.light_min) {
            alarm = true;
        }
    }

    if (g_app.node_b_online && g_app.human) {
        alarm = true;
    }

    return alarm;
}

static void app_update_alarm_state(bool *need_redraw)
{
    uint8_t alarm_now = app_evaluate_alarm() ? 1 : 0;

    if (g_app.alarm_on != alarm_now) {
        g_app.alarm_on = alarm_now;
        if (need_redraw != NULL) {
            *need_redraw = true;
        }
    }

    app_service_apply_alarm_output();
}

static void app_process_espnow_packet(const espnow_packet_t *pkt, bool *need_redraw)
{
    TickType_t now_tick = xTaskGetTickCount();

    if (pkt == NULL || need_redraw == NULL) {
        return;
    }

    /*
     * node_id = 1 -> NodeA
     * msg_type = 1 -> 环境数据
     */
    if (pkt->node_id == 1 && pkt->msg_type == 1)
    {
        if (g_app.temp != pkt->temp ||
            g_app.humi != pkt->humi ||
            g_app.light != pkt->light ||
            g_app.node_a_online == 0)
        {
            g_app.temp = pkt->temp;
            g_app.humi = pkt->humi;
            g_app.light = pkt->light;
            g_app.node_a_online = 1;
            *need_redraw = true;
        }

        s_node_a_last_rx_tick = now_tick;
    }
    /*
     * node_id = 2 -> NodeB
     * msg_type = 2 -> 人体状态
     */
    else if (pkt->node_id == 2 && pkt->msg_type == 2)
    {
        if (g_app.human != pkt->human ||
            g_app.node_b_online == 0)
        {
            g_app.human = pkt->human;
            g_app.node_b_online = 1;
            *need_redraw = true;
        }

        s_node_b_last_rx_tick = now_tick;
    }
    /*
     * 心跳包
     */
    else if (pkt->node_id == 1 && pkt->msg_type == 3)
    {
        if (g_app.node_a_online == 0)
        {
            g_app.node_a_online = 1;
            *need_redraw = true;
        }

        s_node_a_last_rx_tick = now_tick;
    }
    else if (pkt->node_id == 2 && pkt->msg_type == 3)
    {
        if (g_app.node_b_online == 0)
        {
            g_app.node_b_online = 1;
            *need_redraw = true;
        }

        s_node_b_last_rx_tick = now_tick;
    }
}

void app_service_init(void)
{
    app_load_settings();
    const char *mac_now;

    g_app.wifi_ok = wifi_service_is_connected();
    strncpy(g_app.wifi_ip, wifi_service_get_ip_str(), sizeof(g_app.wifi_ip) - 1);
    g_app.wifi_ip[sizeof(g_app.wifi_ip) - 1] = '\0';
    mac_now = wifi_service_get_mac_str();
    strncpy(g_app.wifi_mac, mac_now, sizeof(g_app.wifi_mac) - 1);
    g_app.wifi_mac[sizeof(g_app.wifi_mac) - 1] = '\0';
    g_app.wifi_channel = wifi_service_get_channel();

    /* 初始先认为离线，等收到包再置在线 */
    g_app.node_a_online = 0;
    g_app.node_b_online = 0;

    s_node_a_last_rx_tick = 0;
    s_node_b_last_rx_tick = 0;

    app_update_alarm_state(NULL);
}

void app_service_apply_alarm_output(void)
{
    buzzer_set(g_app.buzzer_on);
}

esp_err_t app_service_save_settings(void)
{
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open("app_cfg", NVS_READWRITE, &nvs);

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "open app_cfg for write failed: %s", esp_err_to_name(ret));
        return ret;
    }

    app_settings_nvs_t settings = {
        .magic = APP_SETTINGS_MAGIC,
        .temp_max = g_app.temp_max,
        .humi_max = g_app.humi_max,
        .light_min = g_app.light_min,
        .buzzer_on = g_app.buzzer_on ? 1 : 0,
    };

    ret = nvs_set_blob(nvs, "settings", &settings, sizeof(settings));
    if (ret == ESP_OK) {
        ret = nvs_commit(nvs);
    }

    nvs_close(nvs);

    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "save settings failed: %s", esp_err_to_name(ret));
    }

    return ret;
}

bool app_service_update(void)
{
    bool need_redraw = false;
    uint8_t wifi_now = wifi_service_is_connected();
    const char *ip_now = wifi_service_get_ip_str();
    const char *mac_now = wifi_service_get_mac_str();
    uint8_t channel_now = wifi_service_get_channel();
    TickType_t now_tick = xTaskGetTickCount();

    /* 同步 WiFi 状态 */
    if (g_app.wifi_ok != wifi_now)
    {
        g_app.wifi_ok = wifi_now;
        need_redraw = true;
    }

    if (strncmp(g_app.wifi_ip, ip_now, sizeof(g_app.wifi_ip)) != 0)
    {
        strncpy(g_app.wifi_ip, ip_now, sizeof(g_app.wifi_ip) - 1);
        g_app.wifi_ip[sizeof(g_app.wifi_ip) - 1] = '\0';
        need_redraw = true;
    }

    if (strncmp(g_app.wifi_mac, mac_now, sizeof(g_app.wifi_mac)) != 0)
    {
        strncpy(g_app.wifi_mac, mac_now, sizeof(g_app.wifi_mac) - 1);
        g_app.wifi_mac[sizeof(g_app.wifi_mac) - 1] = '\0';
        need_redraw = true;
    }

    if (g_app.wifi_channel != channel_now)
    {
        g_app.wifi_channel = channel_now;
        need_redraw = true;
    }

    /* 处理真实 ESPNOW 数据队列 */
    espnow_packet_t pkt;
    while (espnow_service_poll_packet(&pkt))
    {
        app_process_espnow_packet(&pkt, &need_redraw);
    }

    /* 节点在线超时判断 */
    if (g_app.node_a_online &&
        (now_tick - s_node_a_last_rx_tick > pdMS_TO_TICKS(NODE_OFFLINE_TIMEOUT_MS)))
    {
        g_app.node_a_online = 0;
        need_redraw = true;
    }

    if (g_app.node_b_online &&
        (now_tick - s_node_b_last_rx_tick > pdMS_TO_TICKS(NODE_OFFLINE_TIMEOUT_MS)))
    {
        g_app.node_b_online = 0;
        need_redraw = true;
    }

    app_update_alarm_state(&need_redraw);

    return need_redraw;
}
