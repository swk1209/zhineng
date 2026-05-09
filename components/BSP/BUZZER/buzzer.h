#ifndef __BUZZER_H
#define __BUZZER_H

#include <stdint.h>
#include "driver/gpio.h"

#define BUZZER_GPIO_PIN    GPIO_NUM_9

void buzzer_init(void);
void buzzer_set(uint8_t on);

#endif
