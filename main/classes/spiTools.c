#include "spiTools.h"

void spiInit(spi_host_device_t host, int mosi, int miso, int clk) {
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = mosi,
        .miso_io_num = miso,
        .sclk_io_num = clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096
    };
    spi_bus_initialize(host, &bus_cfg, SPI_DMA_CH_AUTO);
}