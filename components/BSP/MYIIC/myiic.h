#ifndef __MYIIC_H
#define __MYIIC_H

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"

/* I2C0 引脚与相关参数定义 */
#define IIC_NUM_PORT        I2C_NUM_0        /* IIC0 */
#define IIC_SPEED_CLK       400000           /* 速率 400K */
#define IIC_SDA_GPIO_PIN    GPIO_NUM_38      /* IIC0_SDA 引脚 */
#define IIC_SCL_GPIO_PIN    GPIO_NUM_48      /* IIC0_SCL 引脚 */

/* I2C1 引脚定义（预留给 ES8311 等外设） */
#define IIC1_NUM_PORT       I2C_NUM_1
#define IIC1_SDA_GPIO_PIN   GPIO_NUM_4
#define IIC1_SCL_GPIO_PIN   GPIO_NUM_5

/* 全局总线句柄 */
extern i2c_master_bus_handle_t bus_handle;   /* I2C0 总线句柄 */
extern i2c_master_bus_handle_t bus_handle1;  /* I2C1 总线句柄 */

/* 函数声明 */
esp_err_t myiic_init(void);    /* 初始化 I2C0 */
esp_err_t myiic_init1(void);   /* 初始化 I2C1 */

#endif