#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "touch.h"
#include "led.h"
#include "myiic.h"
#include "my_spi.h"
#include "spilcd.h"
#include "xl9555.h"
#include "ui.h"
#include "wifi_service.h"
#include "app_service.h"
#include "espnow_service.h"

void app_main(void)
{
    esp_err_t ret;
    uint8_t last_status[CT_MAX_TOUCH] = {0};
    uint8_t t;

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    led_init();
    my_spi_init();
    myiic_init();
    xl9555_init();
    spilcd_init();

    /* 先显示启动页，避免白屏 */
    spilcd_clear(WHITE);
    spilcd_show_string(25, 90, 320, 24, 24, "Smart Env Monitor", RED);
    spilcd_show_string(70, 130, 320, 16, 16, "System Booting...", BLUE);

    tp_dev.init();

    /* 初始化 WiFi */
    wifi_service_init();

    /* 初始化 ESPNOW */
    espnow_service_init();

    /* 初始化 APP 状态同步层 */
    app_service_init();

    /* 首次进入页面，整页绘制一次 */
    g_need_redraw = 1;
    ui_draw_page();

    while (1)
    {
        /* 更新应用状态：
         * 数据变化时只做当前页局部刷新
         */
        if (app_service_update())
        {
            ui_refresh_current_page();
        }

        /* 扫描触摸 */
        tp_dev.scan();

        for (t = 0; t < CT_MAX_TOUCH; t++)
        {
            if (tp_dev.sta & (1 << t))
            {
                if (last_status[t] == 0)
                {
                    ui_touch_process(tp_dev.x[t], tp_dev.y[t]);
                    last_status[t] = 1;
                    LED0_TOGGLE();
                }
            }
            else
            {
                if (last_status[t] == 1)
                {
                    last_status[t] = 0;
                }
            }
        }

        /* 只有切页时才整页重绘 */
        if (g_need_redraw)
        {
            ui_draw_page();
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}