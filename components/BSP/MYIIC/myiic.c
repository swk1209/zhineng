#include "myiic.h"

i2c_master_bus_handle_t bus_handle = NULL;   /* I2C0 总线句柄 */
i2c_master_bus_handle_t bus_handle1 = NULL;  /* I2C1 总线句柄 */

/**
 * @brief       初始化 I2C0
 * @param       无
 * @retval      ESP_OK: 初始化成功
 */
esp_err_t myiic_init(void)
{
    if (bus_handle != NULL) {
        return ESP_OK;
    }

    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,       /* 时钟源 */
        .i2c_port = IIC_NUM_PORT,                /* I2C 端口 */
        .scl_io_num = IIC_SCL_GPIO_PIN,          /* SCL 引脚 */
        .sda_io_num = IIC_SDA_GPIO_PIN,          /* SDA 引脚 */
        .glitch_ignore_cnt = 7,                  /* 故障过滤周期 */
        .flags.enable_internal_pullup = true,    /* 内部上拉 */
    };

    esp_err_t ret = i2c_new_master_bus(&i2c_bus_config, &bus_handle);
    if (ret != ESP_OK) {
        return ret;
    }

    return ESP_OK;
}

/**
 * @brief       初始化 I2C1（预留给 ES8311 等外设）
 * @param       无
 * @retval      ESP_OK: 初始化成功
 */
esp_err_t myiic_init1(void)
{
    if (bus_handle1 != NULL) {
        return ESP_OK;
    }

    i2c_master_bus_config_t bus_cfg1 = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = IIC1_NUM_PORT,
        .scl_io_num = IIC1_SCL_GPIO_PIN,
        .sda_io_num = IIC1_SDA_GPIO_PIN,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t ret = i2c_new_master_bus(&bus_cfg1, &bus_handle1);
    if (ret != ESP_OK) {
        return ret;
    }

    return ESP_OK;
}