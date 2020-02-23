#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "lvgl/lvgl.h"
#include "esp_freertos_hooks.h"


#include "disp_spi.h"
#include "disp_driver.h"

#include "tp_spi.h"
#include "xpt2046.h"
#include "tp_i2c.h"
#include "ft6x36.h"
#include "reppanel.h"

static void IRAM_ATTR lv_tick_task(void);

void app_main() {
    lv_init();

    disp_spi_init();
    disp_driver_init();

#if CONFIG_LVGL_TOUCH_CONTROLLER == 1
    tp_spi_init();
    xpt2046_init();
#elif CONFIG_LVGL_TOUCH_CONTROLLER == 2
    ft6x06_init(FT6236_I2C_SLAVE_ADDR);
#endif

    static lv_color_t buf1[DISP_BUF_SIZE];
    static lv_color_t buf2[DISP_BUF_SIZE];
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

#if CONFIG_LVGL_TOUCH_CONTROLLER == 1
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = xpt2046_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
#elif CONFIG_LVGL_TOUCH_CONTROLLER == 2
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = ft6x36_touch_xy;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
#endif

    esp_register_freertos_tick_hook(lv_tick_task);

    rep_panel_ui_create();

    while (1) {
        vTaskDelay(1);
        lv_task_handler();
    }
}

static void IRAM_ATTR lv_tick_task(void) {
    lv_tick_inc(portTICK_RATE_MS);
}
