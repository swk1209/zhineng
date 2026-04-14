#ifndef __ESPNOW_SERVICE_H
#define __ESPNOW_SERVICE_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

typedef struct __attribute__((packed)) {
    uint8_t  node_id;
    uint8_t  msg_type;
    float    temp;
    float    humi;
    float    light;
    uint8_t  human;
    uint32_t tick;
} espnow_packet_t;

esp_err_t espnow_service_init(void);
esp_err_t espnow_service_add_peer(const uint8_t *peer_mac);
bool espnow_service_poll_packet(espnow_packet_t *pkt);

#endif