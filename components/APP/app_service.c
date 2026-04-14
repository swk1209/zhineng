#include "app_service.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_data.h"
#include "wifi_service.h"
#include "espnow_service.h"

/* 节点离线超时时间 */
#define NODE_OFFLINE_TIMEOUT_MS   10000

static TickType_t s_node_a_last_rx_tick = 0;
static TickType_t s_node_b_last_rx_tick = 0;

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
    g_app.wifi_ok = wifi_service_is_connected();
    strncpy(g_app.wifi_ip, wifi_service_get_ip_str(), sizeof(g_app.wifi_ip) - 1);
    g_app.wifi_ip[sizeof(g_app.wifi_ip) - 1] = '\0';

    /* 初始先认为离线，等收到包再置在线 */
    g_app.node_a_online = 0;
    g_app.node_b_online = 0;

    s_node_a_last_rx_tick = 0;
    s_node_b_last_rx_tick = 0;
}

bool app_service_update(void)
{
    bool need_redraw = false;
    uint8_t wifi_now = wifi_service_is_connected();
    const char *ip_now = wifi_service_get_ip_str();
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

    return need_redraw;
}