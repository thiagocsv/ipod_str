#include "tasks.h"

QueueHandle_t cmdQueue;
SemaphoreHandle_t sdsemphr;
EventGroupHandle_t audioEvents;
EventGroupHandle_t uiEvents;

lcdDevice display;
sdDevice sdcard;
audioClass player;


void app_main(void)
{
    cmdQueue = xQueueCreate(10, sizeof(ui_commands_t));
    sdsemphr = xSemaphoreCreateMutex();
    audioEvents = xEventGroupCreate();
    uiEvents = xEventGroupCreate();

    if(cmdQueue == NULL || sdsemphr == NULL || audioEvents == NULL || uiEvents == NULL)
        return;

    xTaskCreatePinnedToCore
    (
        taskInitInterfaces,
        "General_Setup",
        4096,
        NULL,
        5,
        NULL,
        1
    );
}
