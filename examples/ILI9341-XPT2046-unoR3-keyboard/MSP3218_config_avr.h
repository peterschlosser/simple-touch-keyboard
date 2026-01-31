#ifndef _MSP3218_CONFIG_H
#define _MSP3218_CONFIG_H

#define TFT_CLK 13  // standard config
#define TFT_MISO 12 // standard config
#define TFT_MOSI 11 // standard config
#define TFT_CS 10   // standard config
#define TFT_DC 8    // data/command
#define TFT_RST 7   // MSP3218 needs reset
#define TFT_LED 9   // LED backlight (PWM capable)
#define TFT_SPI_SPD (4L * 1024 * 1024)

#define TS_CLK 13  // shared with TFT
#define TS_MISO 12 // shared with TFT
#define TS_MOSI 11 // shared with TFT
#define TS_CS 6    // chip/slave select
#define TS_IRQ -1  // unimplemented
#define TS_SPI_SPD (2L * 1024 * 1024)

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