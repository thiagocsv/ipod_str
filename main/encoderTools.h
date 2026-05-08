#ifndef ENCODERTOOLS_H
#define ENCODERTOOLS_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "btnTools.h"

#define ENC_CLK 35
#define ENC_DT  36

extern QueueHandle_t cmdQueue;

void encoder_init(void);

#endif
