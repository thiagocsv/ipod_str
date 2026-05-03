#ifndef SPITOOLS_H
#define SPITOOLS_H

#include "driver/spi_common.h"
#include "driver/spi_master.h"

typedef struct {
    spi_host_device_t host_id;
} spiBus;

void spiInit(spi_host_device_t host, int mosi, int miso, int clk);

#endif
