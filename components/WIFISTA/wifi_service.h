#ifndef __WIFI_SERVICE_H
#define __WIFI_SERVICE_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t wifi_service_init(void);
bool wifi_service_is_connected(void);
const char *wifi_service_get_ip_str(void);
const char *wifi_service_get_mac_str(void);
uint8_t wifi_service_get_channel(void);

#ifdef __cplusplus
}
#endif

#endif
