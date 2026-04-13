#include "app_data.h"

app_data_t g_app = {
    .temp = 26.5f,
    .humi = 58.0f,
    .light = 123.0f,
    .human = 1,

    .node_a_online = 1,
    .node_b_online = 1,
    .wifi_ok = 0,
    .mqtt_ok = 0,
    .alarm_on = 0,

    .wifi_ip = "0.0.0.0",

    .temp_max = 30.0f,
    .humi_max = 80.0f,
    .light_min = 50.0f,

    .buzzer_on = 1,
};