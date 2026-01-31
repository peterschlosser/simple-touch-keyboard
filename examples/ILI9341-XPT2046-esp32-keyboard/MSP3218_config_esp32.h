// GPIO Pin Configuration for the ESP32-S3-DevKitC-1 (ESP32-S3-N16R8)
// this configuration places all three systems (display, touch, sd-card) on
// same SPI bus and uses 9 GPIO pins.
// compare to the pin configuration suggested by the LCDWiki documentation
// for the MSP3218 module using as many as 16 GPIO pins.
#ifndef _MSP3218_CONFIG_H
#define _MSP3218_CONFIG_H

// ILI9341 SPI TFT LCD Controller
#define TFT_CLK 12  // standard SPI SCK
#define TFT_MISO 13 // standard SPI MISO
#define TFT_MOSI 11 // standard SPI MOSI
#define TFT_CS 10   // chip/slave select
#define TFT_DC 7    //
#define TFT_LED 5   //
#define TFT_RST 6   // MSP3218 wants a hard reset

// XPT2046 SPI Touch Panel Controller
#define TS_CLK 12  // shared with TFT
#define TS_MISO 13 // shared with TFT
#define TS_MOSI 11 // shared with TFT
#define TS_CS 4    // chip/slave select
#define TS_IRQ -1  // unimplemented

// SPI Standard-SD (SDHC) Card Connector
#define SD_CLK 12  // shared with TFT
#define SD_MISO 13 // shared with TFT
#define SD_MOSI 11 // shared with TFT
#define SD_CS 2    // chip/slave select

// XPT2046_Toushscreen_TT allows decent calibration values for differing
// screen rotations.  the values below were dervied using rotation(1) in
// the library TS_DisplayCalibration example.  the values may be tested
// with the TS_DisplayTest example.
#define MY_TS_LR_X 348
#define MY_TS_LR_Y 166
#define MY_TS_UL_X 3954
#define MY_TS_UL_Y 3917
#define MY_TS_THRESH 500 // too low a value generates spurious taps

#endif