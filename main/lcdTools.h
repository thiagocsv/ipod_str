#ifndef LCDTOOLS_H
#define LCDTOOLS_H

#include <stdint.h>
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include <stdbool.h>
#include <string.h>

typedef struct {
    int rs;
    int en;
    int data_pins[8];
} lcdDevice;

void createChar(lcdDevice *self, uint8_t loc, uint8_t map[]);

void setCursor(lcdDevice *self, uint8_t col, uint8_t line);

void lcdInit(lcdDevice *self, int rs, int en, int d0, int dq, int int d2, int d3, int d4, int d5, int d6, int d7);

void lcdWrite(lcdDevice *self, const char *txt, uint8_t col, uint8_t line, uint8_t offset, uint8_t until);

void clearLCD(lcdDevice *self);

#endif
