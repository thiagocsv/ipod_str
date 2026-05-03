#ifndef SDTOOLS_H
#define SDTOOLS_H

#include "esp_err.h"

typedef struct {
    spi_host_device_t host_id;
    int cs_pin;
    const char *mount_point;
} sdDevice;

void sd_create(sdDevice *self, spi_host_device_t host_id, int cs_pin, const char *mount_point);

esp_err_t sd_mount(sdDevice *self);

#endif
