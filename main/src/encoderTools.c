#include "encoderTools.h"

static void IRAM_ATTR encoder_isr_handler(void* arg)
{
    static uint32_t last_isr_time = 0;
    uint32_t current_time = xTaskGetTickCountFromISR();

    if (current_time - last_isr_time < pdMS_TO_TICKS(10)) {
        return;
    }
    last_isr_time = current_time;

    ui_commands_t cmd;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (gpio_get_level(ENC_DT) == 1) {
        cmd = NAV_UP;
    } else {
        cmd = NAV_DOWN;
    }

    xQueueSendFromISR(cmdQueue, &cmd, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

void encoder_init(void)
{
    gpio_set_direction(ENC_CLK, GPIO_MODE_INPUT);
    gpio_set_direction(ENC_DT, GPIO_MODE_INPUT);
    gpio_set_intr_type(ENC_CLK, GPIO_INTR_NEGEDGE);

    gpio_isr_handler_add(ENC_CLK, encoder_isr_handler, NULL);
}
