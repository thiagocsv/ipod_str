#include "sdTools.h"

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

void lcdInit(lcdDevice *self, int rs, int en, int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7)
{
    self->rs = rs;
    self->en = en;
    self->data_pins[0] = d0;
    self->data_pins[1] = d1;
    self->data_pins[2] = d2;
    self->data_pins[3] = d3;
    self->data_pins[4] = d4;
    self->data_pins[5] = d5;
    self->data_pins[6] = d6;
    self->data_pins[7] = d7;

    gpio_set_direction(self->rs, GPIO_MODE_OUTPUT);
    gpio_set_direction(self->en, GPIO_MODE_OUTPUT);
    for(int i=0; i<8; i++) {
        gpio_set_direction(self->data_pins[i], GPIO_MODE_OUTPUT);
    }
}

void lcdWrite(lcdDevice *self, const char *txt, uint8_t x, uint8_t y, uint8_t offset, uint8_t until)
{
    setCursor(self, x, y);

    int char_count = 0;
    int len = strlen(txt);

    if(len > 0)
    {
        int start_pos = offset % (len + 3);

        for(int i=0;i<until;i++)
        {
            int current = start_pos + i;

            if(current < len)
            {
                sendByte(self, txt[current], 1);
                char_count++;
            }

            else
                break;
        }
    }

    while(char_count < until)
    {
        sendByte(self, ' ', 1);
        char_count++;
    }
}

void writeList(uint32_t *w0, uint32_t *w1, bool line)
{
    lcdWrite(&display, w0, 1, 0, 0, 15);
    lcdWrite(&display, w0, 1, 0, 0, 15);

    lcdWrite(&display, ">", 0, line, 0, 1);
}
