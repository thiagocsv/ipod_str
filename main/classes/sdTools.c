#include "sdTools.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"

void sd_create(sdDevice *self, spi_host_device_t host_id, int cs_pin, const char *mount_point) {
    self->host_id = host_id;
    self->cs_pin = cs_pin;
    self->mount_point = mount_point;
    self->card = NULL;
}

esp_err_t sd_mount(sdDevice *self) {
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false, .max_files = 5, .allocation_unit_size = 16 * 1024
    };
    
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = self->host_id;
    
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = self->cs_pin;
    slot_config.host_id = self->host_id;
    
    sdmmc_card_t *card;
    return esp_vfs_fat_sdspi_mount(self->mount_point, &host, &slot_config, &mount_config, &self->card);
}
