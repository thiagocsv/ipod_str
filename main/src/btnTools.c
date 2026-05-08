#include "btnTools.h"

static btn_t buttons[4];

static void IRAM_ATTR btn_isr(void *params)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    btn_t *btn = (btn_t *)params;

    xTimerResetFromISR(btn->debounce_timer, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void hold_btn (TimerHandle_t xTimer)
{
    btn_t *btn = (btn_t *)pvTimerGetTimerID(xTimer);

    xTimerStop(btn->tap_timer, 0);
    btn->tap = 0;
    btn->state = IDLE;

    xQueueSend(cmdQueue, &btn->hold, 0);
}

static void tap_timer(TimerHandle_t xTimer)
{
    btn_t *btn = (btn_t *)pvTimerGetTimerID(xTimer);

    ui_commands_t cmd;

    switch(btn->tap)
    {
        case 1: cmd = cmd_single;
                break;
        case 2: cmd = cmd_double;
                break;
        case 3: cmd = cmd_triple;
                break;
        default: cmd = cmd_triple;
                break;
    }

    xQueueSend(cmdQueue, &cmd, 0);

    btn->tap = 0;
    btn->state = IDLE;
}

static void debounce(TimerHandle_t xTimer)
{
    btn_t *btn = (btn_t *)pvTimerGetTimerID(xTimer);
    int level = gpio_get_level(btn->pin);

    if(!level)
    {
        if(btn->state == IDLE || btn->state == RELEASED)
        {
            btn->state = PRESSED;
            btn->tap += 1;

            xTimerStart(btn->hold, 0);
        }
    }
    else
    {
        if(btn->state == PRESSED)
        {
            xTimerStop(btn->hold, 0);

            btn->state = RELEASED;

            xTimerReset(btn->tap, 0);
        }
    }
}

void btn_manager(void *params)
{
    buttons[0] = (btn_t)
    {
        .pin = BTN_UP,
        .state = IDLE,
        .tap = 0,
        .cmd_single = VOL_UP,
        .cmd_double = VOL_UP,
        .cmd_triple = VOL_UP,
        .hold = NEXT,
    };

    buttons[1] = (btn_t)
    {
        .pin = BTN_PLAY,
        .state = IDLE,
        .tap = 0,
        .cmd_single = PLAY_PAUSE,
        .cmd_double = NEXT,
        .cmd_triple = PREV,
        .hold = STOP,
    };

    buttons[2] = (btn_t)
    {
        .pin = BTN_DOWN,
        .state = IDLE,
        .tap = 0,
        .cmd_single = VOL_DOWN,
        .cmd_double = VOL_DOWN,
        .cmd_triple = VOL_DOWN,
        .hold = PREV
    };

    buttons[3] = (btn_t)
    {
        .pin = ENC_SW,
        .state = IDLE,
        .tap_count = 0,
        .cmd_single = ENTER,
        .cmd_double = RETURN,
        .cmd_triple = RETURN,
        .cmd_hold = RETURN
    }

    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

    for(int i=0;i<4;i++)
    {
        gpio_set_direction(buttons[i].pin, GPIO_MODE_INPUT);

        gpio_set_intr_type(buttons[i].pin, GPIO_INTR_ANYEDGE);

        buttons[i].debounce = xTimerCreate
        (
            "deb",
            pdMS_TO_TICKS(DEB),
            pdFALSE,
            &buttons[i],
            debounce
        );

        buttons[i].tap = xTimerCreate
        (
            "tap",
            pdMS_TO_TICKS(DB_TAP),
            pdFALSE,
            &buttons[i],
            tap_timer
        );

        buttons[i].hold = xTimerCreate
        (
            "hold",
            pdMS_TO_TICKS(HOLD),
            pdFALSE,
            &buttons[i],
            hold_btn
        );

        gpio_isr_handler_add(buttons[i].pin, btn_isr, &buttons[i]);
    }
}
