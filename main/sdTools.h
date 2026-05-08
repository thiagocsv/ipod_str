#ifndef SDTOOLS_H
#define SDTOOLS_H

#include "esp_err.h"
#include "sdmmc_cmd.h"


typedef struct {
    spi_host_device_t host_id;
    int cs_pin;
    const char *mount_point;
    sdmmc_card_t *card;
} sdDevice;

void sd_create(sdDevice *self, spi_host_device_t host_id, int cs_pin, const char *mount_point);

esp_err_t sd_mount(sdDevice *self);

#endif
