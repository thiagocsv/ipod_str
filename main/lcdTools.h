#ifndef LCDTOOLS_H
#define LCDTOOLS_H

#include <stdint.h>

typedef struct {
    int rs;
    int en;
    int data_pins[8];
} lcdDevice;

void createChar(lcdDevice *self, uint8_t loc, uint8_t map[]);

void setCursor(lcdDevice *self, uint8_t col, uint8_t line);

void lcdWrite(lcdDevice *self, const char *txt, uint8_t col, uint8_t line, uint8_t offset, uint8_t until);

void clearLCD(lcdDevice *self);

#endif
