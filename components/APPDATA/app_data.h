#ifndef __APP_DATA_H
#define __APP_DATA_H

#include <stdint.h>

typedef struct {
    float temp;
    float humi;
    float light;
    uint8_t human;

    uint8_t node_a_online;
    uint8_t node_b_online;
    uint8_t wifi_ok;
    uint8_t mqtt_ok;
    uint8_t alarm_on;

    char wifi_ip[16];

    float temp_max;
    float humi_max;
    float light_min;

    uint8_t buzzer_on;
} app_data_t;

extern app_data_t g_app;

#endif