#ifndef __WIFI_SERVICE_H
#define __WIFI_SERVICE_H

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t wifi_service_init(void);
bool wifi_service_is_connected(void);
const char *wifi_service_get_ip_str(void);

#ifdef __cplusplus
}
#endif

#endif