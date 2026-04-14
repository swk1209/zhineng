#include "espnow_service.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"

#define ESPNOW_QUEUE_LEN  10

static const char *TAG = "ESPNOW_SERVICE";

static QueueHandle_t s_espnow_queue = NULL;
static bool s_espnow_inited = false;

typedef struct {
    espnow_packet_t pkt;
} espnow_queue_item_t;

/**
 * @brief ESPNOW 接收回调
 * @note 这里只做数据拷贝和入队，不做耗时操作
 */
static void espnow_recv_cb(const esp_now_recv_info_t *recv_info,
                           const uint8_t *data,
                           int data_len)
{
    if (recv_info == NULL || data == NULL) {
        return;
    }

    if (data_len != sizeof(espnow_packet_t)) {
        return;
    }

    if (s_espnow_queue == NULL) {
        return;
    }

    espnow_queue_item_t item;
    memset(&item, 0, sizeof(item));
    memcpy(&item.pkt, data, sizeof(espnow_packet_t));

    /* 不阻塞，防止回调卡住 */
    xQueueSend(s_espnow_queue, &item, 0);
}

/**
 * @brief 初始化 ESPNOW
 * @note 调用前建议已经完成 wifi_service_init()
 */
esp_err_t espnow_service_init(void)
{
    esp_err_t ret;

    if (s_espnow_inited) {
        return ESP_OK;
    }

    if (s_espnow_queue == NULL) {
        s_espnow_queue = xQueueCreate(ESPNOW_QUEUE_LEN, sizeof(espnow_queue_item_t));
        if (s_espnow_queue == NULL) {
            ESP_LOGE(TAG, "create queue failed");
            return ESP_FAIL;
        }
    }

    ret = esp_now_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_now_init failed: %d", ret);
        return ret;
    }

    ret = esp_now_register_recv_cb(espnow_recv_cb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "register recv cb failed: %d", ret);
        esp_now_deinit();
        return ret;
    }

    s_espnow_inited = true;
    ESP_LOGI(TAG, "espnow init done");

    return ESP_OK;
}

/**
 * @brief 添加一个从节点 peer
 * @param peer_mac 6字节 MAC 地址
 * @retval esp_err_t
 */
esp_err_t espnow_service_add_peer(const uint8_t *peer_mac)
{
    if (peer_mac == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!s_espnow_inited) {
        return ESP_ERR_INVALID_STATE;
    }

    if (esp_now_is_peer_exist(peer_mac)) {
        return ESP_OK;
    }

    esp_now_peer_info_t peer;
    memset(&peer, 0, sizeof(peer));

    memcpy(peer.peer_addr, peer_mac, 6);
    peer.channel = 0;         /* 跟随当前 STA 信道 */
    peer.ifidx = WIFI_IF_STA; /* 主节点走 STA 接口 */
    peer.encrypt = false;     /* 第一版先不加密 */

    esp_err_t ret = esp_now_add_peer(&peer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "add peer failed: %d", ret);
        return ret;
    }

    ESP_LOGI(TAG, "peer added: %02X:%02X:%02X:%02X:%02X:%02X",
             peer_mac[0], peer_mac[1], peer_mac[2],
             peer_mac[3], peer_mac[4], peer_mac[5]);

    return ESP_OK;
}

/**
 * @brief 从队列中取出一包数据
 * @param pkt 输出数据包
 * @retval true  取到数据
 * @retval false 没有数据
 */
bool espnow_service_poll_packet(espnow_packet_t *pkt)
{
    espnow_queue_item_t item;

    if (pkt == NULL) {
        return false;
    }

    if (s_espnow_queue == NULL) {
        return false;
    }

    if (xQueueReceive(s_espnow_queue, &item, 0) == pdTRUE) {
        memcpy(pkt, &item.pkt, sizeof(espnow_packet_t));
        return true;
    }

    return false;
}