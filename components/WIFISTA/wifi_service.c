#include "wifi_service.h"

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "lwip/inet.h"

#define DEFAULT_SSID        "OPPO"
#define DEFAULT_PWD         "88888888"

#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

static const char *TAG = "WIFI_SERVICE";

static EventGroupHandle_t s_wifi_event = NULL;
static bool s_wifi_connected = false;
static int s_retry_num = 0;
static char s_ip_str[16] = "0.0.0.0";

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "WiFi connecting...");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(TAG, "WiFi AP connected");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        s_wifi_connected = false;
        memset(s_ip_str, 0, sizeof(s_ip_str));
        strcpy(s_ip_str, "0.0.0.0");

        if (s_retry_num < 20)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGW(TAG, "Retry to connect to AP, attempt %d", s_retry_num);
        }
        else
        {
            xEventGroupSetBits(s_wifi_event, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Connect to AP failed");
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        s_retry_num = 0;
        s_wifi_connected = true;
        xEventGroupSetBits(s_wifi_event, WIFI_CONNECTED_BIT);

        snprintf(s_ip_str, sizeof(s_ip_str), IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Got IP: %s", s_ip_str);
    }
}

esp_err_t wifi_service_init(void)
{
    static bool s_inited = false;
    static esp_netif_t *sta_netif = NULL;

    if (s_inited)
    {
        return ESP_OK;
    }

    if (s_wifi_event == NULL)
    {
        s_wifi_event = xEventGroupCreate();
        if (s_wifi_event == NULL)
        {
            return ESP_FAIL;
        }
    }

    esp_err_t ret;

    ret = esp_netif_init();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
    {
        return ret;
    }

    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
    {
        return ret;
    }

    sta_netif = esp_netif_create_default_wifi_sta();
    if (sta_netif == NULL)
    {
        ESP_LOGE(TAG, "Failed to create default wifi sta");
        return ESP_FAIL;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,
                                               ESP_EVENT_ANY_ID,
                                               &wifi_event_handler,
                                               NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                               IP_EVENT_STA_GOT_IP,
                                               &wifi_event_handler,
                                               NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    memcpy(wifi_config.sta.ssid, DEFAULT_SSID, strlen(DEFAULT_SSID));
    memcpy(wifi_config.sta.password, DEFAULT_PWD, strlen(DEFAULT_PWD));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(15000)
    );

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "Successfully connected to AP");
        ESP_LOGI(TAG, "SSID: %s", DEFAULT_SSID);
        ESP_LOGI(TAG, "IP  : %s", s_ip_str);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGE(TAG, "Failed to connect to AP");
        ESP_LOGE(TAG, "SSID: %s", DEFAULT_SSID);
    }
    else
    {
        ESP_LOGW(TAG, "WiFi init wait timeout");
    }

    s_inited = true;
    return ESP_OK;
}

bool wifi_service_is_connected(void)
{
    return s_wifi_connected;
}

const char *wifi_service_get_ip_str(void)
{
    return s_ip_str;
}