#include "tasks.h"

void taskInitInterfaces(void *params)
{
    spiInit(SPI_HOST, 23, 19, 18);
    sd_create(&sdcard, SPI_HOST, 5, "/sdcard");
    sd_mount(&sdcard);

    buildPlayer(&player);
    btn_manager(NULL);

    xSemaphoreGive(sdsemphr);

    xTaskCreatePinnedToCore(taskCmd_routing, "router", 2048, NULL, 4, NULL, 0);
    xTaskCreatePinnedToCore(taskUI, "ui", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(taskAudio, "audio", 8192, NULL, 5, NULL, 1);

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
    while(1)
    {
        EventBits_t bits = xEventGroupWaitBits
        (
            audioEvents,
            EVT_PLAY_PAUSE | EVT_NEXT | EVT_PREV | EVT_STOP,
            pdTRUE,
            pdFALSE,
            pdMS_TO_TICKS(100);
        );

        if(bits)
        {
            if(bits & EVT_PLAY_PAUSE)
            {
                int val = (player->isPlaying << 1) | player->isPaused;

                switch(val)
                {
                    case 0b00:
                        if(xSemaphoreTake(sdsemphr, pdMS_TO_TICKS(500) == pdTRUE))
                        {
                            playSound(&player, "/sdcard/music/");
                            xSemaphoreGive(sdsemphr);
                        }
                        break;
                    case 0b10:
                        pauseSound(&player);
                        break;
                    case 0b11:
                        resumeSound(&player);
                        break;
                }
            }

            if(bits & EVT_STOP)
                stopSound(&player);
        }

        if(player->isPlaying && !player->isPaused)
            if(checkSongFinished(&player))
            {
                continue;
            }
    }
}

void taskUI(void *params)
{
    ui_state_t uistate = UI_MAIN;

    clearLCD(&display);
    lcdWrite(&display, "iPod on ESP32", 0, 0, 0, 16);
    lcdWrite(&display, "Starting...", 0, 1, 0, 16);
    vTaskDelay(pd_MS_TO_TICKS(1500));

    while(1)
    {
        EventBits_t bits = xEventGroupWaitBits(uiEvents, EVT_ENTER | EVT_RETURN, pdTRUE, pdFALSE, portMAX_DELAY);

        switch(uistate)
        {
            case UI_MAIN:

            case UI_LIST:

            case UI_PLAYING:
        }
    }
}
