#include "buzzer.h"

#include "esp_err.h"

void buzzer_init(void)
{
    gpio_config_t gpio_init_struct = {0};

    gpio_init_struct.intr_type = GPIO_INTR_DISABLE;
    gpio_init_struct.mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_init_struct.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_init_struct.pin_bit_mask = 1ull << BUZZER_GPIO_PIN;

    ESP_ERROR_CHECK(gpio_config(&gpio_init_struct));
    buzzer_set(0);
}

void buzzer_set(uint8_t on)
{
    int level = on ? 0 : 1;

    gpio_set_direction(BUZZER_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BUZZER_GPIO_PIN, level);
}
