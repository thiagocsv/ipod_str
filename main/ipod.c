#include "spiTools.h"
#include "audioTools.h"
#include "lcdTools.h"

#define SDA
#define SCL

#define MOSI
#define MISO
#define CLK
#define CS

EventGroupHandle_t show;
lcdDevice display;
sdDevice sdcard;
audioClass player;

void app_main(void)
{
    sendByte(&lcd, 0x0C, 0);

    xTaskCreatePinnedToCore
    (
        initInterfaces,
        "interfaces_initializer",
        1024,
        NULL,
        1,
        NULL,
        0
    );


}
