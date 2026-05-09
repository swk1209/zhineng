#ifndef __APP_SERVICE_H
#define __APP_SERVICE_H

#include <stdbool.h>
#include "esp_err.h"

void app_service_init(void);
bool app_service_update(void);
void app_service_apply_alarm_output(void);
esp_err_t app_service_save_settings(void);

#endif
