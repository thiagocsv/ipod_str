#include "tasks.h"

void taskInitInterfaces(void *params)
{
    spiInit();

    sd_create();
    sd_mount();

    buildPlayer(&player);

    btn_manager();

    xSemaphoreGive(sdsemphr);

    vTaskDelete(NULL);
}

void taskCmd_routing(void *params)
{
    ui_commands_t cmd;

    while(1)
    {
        if(xQueueReceive(cmdQueue, &cmd, portMAX_DELAY) == pdTRUE)
        {
            EventBits_t audio = audioBits(cmd);
            EventBits_t ui = uiBits(cmd);

            if(audio)
                xEventGroupSetBits(audioEvents, audio);
            if(ui)
                xEventGroupSetBits(uiEvents, ui);
        }
    }
}



void taskAudio(void *params)
{
    xSemaphoreTake(sdsemphr, portMAX_DELAY);

    TickType_t xLastWake = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(20);

    EventBits_t bits = xEventGroupWaitBits(
        audioEvents,
        EVT_PLAY_PAUSE | EVT_NEXT | EVT_PREV | EVT_STOP,
        pdTRUE,
        pdFALSE,
        0
    );

    while(1)
    {
        if(xSemaphoreTake(sdsemphr, 0) == pdTRUE)
        {
            stopSound(&player);
            sd_mount(&sd_card);
            buildPlayer(&player);


            if(bits & EVT_PLAY_PAUSE)
            {
                int val = (player->isPlaying << 1) | player->isPaused;

                switch(val)
                {
                    case 0b00:  playSound(player, "ainda vem");
                                break;
                    case 0b10:  pauseSound(player);
                                break;
                    case 0b11:  resumeSound(player);
                }
            }

            if(bits & EVT_STOP)
            {
                stopSound(player);
            }
        }

        vTaskDelayUntil(&xLastWake, period);
    }
}
