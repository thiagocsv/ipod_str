#ifndef TASKS_H
#define TASKS_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "lcdTools.h"
#include "sdTools.h"
#include "audioTools.h"
#include "btnTools.h"
#include "libManager.h"

#define EVT_PLAY_PAUSE (1 << 0)
#define EVT_NEXT (1 << 1)
#define EVT_PREV (1 << 2)
#define EVT_STOP (1 << 3)

#define EVT_VOL_DOWN (1 << 0)
#define EVT_VOL_UP (1 << 1)
#define EVT_ENTER (1 << 2)
#define EVT_RETURN (1 << 3)
#define EVT_NAV_UP (1 << 4)
#define EVT_NAV_DOWN (1 << 5)

extern lcdDevice display;
extern sdDevice sdcard;
extern audioClass player;
extern QueueHandle_t cmdQueue;
extern SemaphoreHandle_t sdsemphr;
extern EventGroupHandle_t audioEvents;
extern EventGroupHandle_t uiEvents;

typedef enum
{
    UI_MAIN,
    UI_LIST,
    UI_PLAYING
} ui_state_t;

static inline EventBits_t audioBits(ui_commands_t cmd)
{
    switch (cmd)
    {
        case PLAY_PAUSE:    return EVT_PLAY_PAUSE;
        case NEXT:          return EVT_NEXT;
        case PREV:          return EVT_PREV;
        case STOP:          return EVT_STOP;
        default:            return 0;
    }
}

static inline EventBits_t uiBits(ui_commands_t cmd)
{
    switch (cmd)
    {
        case VOL_UP:       return EVT_VOL_UP;
        case VOL_DOWN:     return EVT_VOL_DOWN;
        case ENTER:        return EVT_ENTER;
        case RETURN:       return EVT_RETURN;
        case NAV_UP:       return EVT_NAV_UP;
        case NAV_DOWN:     return EVT_NAV_DOWN;
        default:           return 0;
    }
}

void taskInitInterfaces(void *params);

void taskCmd_routing(void *params);

void taskUI(void *params);

void taskAudio(void *params);

#endif
