#include "tasks.h"

void taskInitInterfaces(void *params)
{
    lcdInit();

    spiInit(SPI_HOST, 23, 19, 18);

    sd_create(&sdcard, SPI_HOST, 5, "/sdcard");
    sd_mount(&sdcard);

    buildPlayer(&player);

    btn_manager(NULL);
    encoder_init();

    xSemaphoreGive(sdsemphr);

    xTaskCreatePinnedToCore(taskCmd_routing, "router", 2048, NULL, 4, NULL, 0);
    xTaskCreatePinnedToCore(taskUI, "ui", 4096, NULL, 3, NULL, 0);
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
    int list_sel = 0;

    clearLCD(&display);
    lcdWrite(&display, "iPod on ESP32", 0, 0, 0, 16);
    lcdWrite(&display, "Starting...", 0, 1, 0, 16);
    vTaskDelay(pd_MS_TO_TICKS(1500));

    while(1)
    {
        EventBits_t bits = xEventGroupWaitBits(uiEvents, EVT_ENTER | EVT_RETURN | EVT_NAV_UP | EVT_NAV_DOWN, pdTRUE, pdFALSE, pdMS_TO_TICKS(100));

        switch(uistate)
        {
            case UI_MAIN:
                static short int main_opt = 0b1100, dex = 0;

                const char *text[4] = {"Songs", "Artists", "Albums", "Genres"};
                bool y = 0;

                if(bits)
                {
                    if(bits & EVT_NAV_DOWN)
                    {
                        if(main_opt != 0b0011)
                            main_opt >>= 1;

                        if(dex >= 0)
                            dex++;
                    }
                    else if(bits & EVT_NAV_UP)
                    {
                        if(main_opt != 0b1100)
                            main_opt <<= 1;

                        if(dex <= 3)
                            dex--;
                    }
                    else if(bits & EVT_ENTER)
                    {
                        uistate = UI_LIST;
                        list_sel = dex;
                    }
                    else if(bits & EVT_RETURN)
                    {
                        uistate = UI_PLAYING;
                    }

                    for(int i=0; i<4;i++)
                    {
                        if(main_opt & (1<<i))
                        {
                            lcdWrite(&display, text[i], 1, y, 0, 16);
                            y != y;
                        }
                    }

                    lcdWrite(&display, ">", 0, dex%2, 0, 1);
                }
                break;

            case UI_LIST:

                static unsigned short int w0 = 0, w1 = 1;
                uint32_t filtr[10];

                int list_size = totalLines(files[list_sel]);

                loadBuffer(files[list_sel], filtr, 10, 0);
                writeList(filtr[w0], filtr[w1], 0);

                if(bits)
                {
                    if(bits & EVT_NAV_DOWN)
                    {

                    }
                    else if(bits & EVT_NAV_UP)
                    {

                    }
                    else if(bits & EVT_ENTER)
                    {

                    }
                    else if(bits & EVT_RETURN)
                    {
                        uistate = UI_MAIN;
                    }


                }

            case UI_PLAYING:
        }
    }
}
