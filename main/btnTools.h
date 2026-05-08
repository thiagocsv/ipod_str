#ifndef BTNTOOLS_H
#define BTNTOOLS_H

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "freertos/queue.h"
#include <stdbool.h>
#include <stdint.h>

#define BTN_PLAY 32
#define BTN_UP 33
#define BTN_DOWN 25
#define ENC_SW 34

#define DEB 50
#define DB_TAP 300
#define HOLD 800

typedef enum
{
    PLAY_PAUSE,
    NEXT,
    PREV,
    STOP,
    VOL_UP,
    VOL_DOWN,
    ENTER,
    RETURN,
    NAV_UP,
    NAV_DOWN
} ui_commands_t;

typedef enum
{
    IDLE,
    PRESSED,
    RELEASED
} btn_state_t;

typedef struct
{
    int pin;
    btn_state_t state;
    int tap_count;
    Timerhandle_t debounce;
    Timerhandle_t tap;
    Timerhandle_t hold;
    ui_commands_t cmd_single;
    ui_commands_t cmd_double;
    ui_commands_t cmd_triple;
} btn_t;

extern QueueHandle_t cmdQueue;

void btn_manager(void *params);

#endif
