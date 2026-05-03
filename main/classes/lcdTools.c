#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include <stdbool.h>
#include <string.h>

static void sendByte(lcdDevice *self, uint8_t value, bool rs)
{
    gpio_set_level(self->rs, rs);

    for(int i=0; i<8; i++)
    {
        gpio_set_level(self->data_pins[i], (value >> i) & 0x01);
    }

    gpio_set_level(self->en, 1);
    esp_rom_delay_us(1); 
    gpio_set_level(self->en, 0);

    esp_rom_delay_us(50);
}

void createChar(lcdDevice *self, uint8_t loc, uint8_t map[])
{
    loc &= 0x07;

    sendByte(self, 0x40 | (loc << 3), 0);

    for(int i=0; i<8; i++)
        sendByte(self, map[i], 1);

    sendByte(self, 0x80, 0);
}

void clearLCD(lcdDevice *self)
{
    sendByte(self, 0x01, 0);
    esp_rom_delay_us(2000);
}

void setCursor(lcdDevice *self, uint8_t col, uint8_t line)
{
    uint8_t address;
    if(line)
        address = 0xC0 + col;
    else
        address = 0x80 + col;

    sendByte(self, address, 0);
}

void lcdWrite(lcdDevice *self, const char *txt, uint8_t col, uint8_t line, uint8_t offset, uint8_t until)
{
    setCursor(self, col, line);

    int size = strlen(txt);

    if(size <= until)
        for(int i=0; i<size; i++)
            sendByte(self, txt[i], 1);

    for(int i=0; i<until; i++)
    {
        int index = (offset + i) % size;
        sendByte(self, txt[index], 1);
    }
}
