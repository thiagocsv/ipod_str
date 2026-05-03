idf_component_register(
    SRCS "ipod.c" 
         "i2cTools.c" 
         "spiTools.c" 
         "lcdTools.c" 
         "sdTools.c" 
         "audioTools.c"
    INCLUDE_DIRS "."
    REQUIRES fatfs vfs sdmmc esp_driver_spi esp_driver_i2c
)