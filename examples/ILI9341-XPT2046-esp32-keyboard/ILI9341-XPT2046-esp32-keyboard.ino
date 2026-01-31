//
// a simple touch keyboard for low-resolution displays and ESP32 processors.
// author: pschlosser707@gmail.com
// ver 1 - initial release
//
// optimized for:
// - the ESP32-S3 DevKitC-1.
// - 240 x 320 TFT LCD displays, such as the MSP3218 TFT SPI "Red Display"
//   using the ILI9341 and XPT2046 controllers.
//
// supports:
// - screen rotation,
// - 93-symbol U.S. keyboard,
// - repeating backspace
//
// overview:
// For projects in need of alpha-numeric user input via touch panel on smaller
// lightweight MCUs, this project demonstrates a fully functioning keyboard
// similar to those presented by mobile devices, and offers a means to get
// user input to their application.
// For the majority of MCU-controlled touch displays, the LvGL Library is a
// common solution to present graphical user interfaces and, where needed, the
// alpha-numeric touch keyboard. But the memory and speed requirements for
// LvGL exceed those of the more lightweight MCU, such as the Arduino Uno R3,
// with its limited speed and memory.  Smaller MCUs are limited in the display
// resolution they can realistically operate.  Using smaller color displays,
// such as the 240x320 TFT LCD supported by the ILI9341 and ST7789 display
// driver ICs, the limited resolution can provide challenges presenting the
// fully functiononal alpha-numeric keyboard.
// As with any software solution, "there is more than one way to skin a cat."
// This project simply presents one method to resolve the issue, and may help
// the consumer know what is possible, and give them a head-start when in need
// of user keybaord input.
//
// theory of operation:
// Presented here are 4 keypad (keyboard) layouts, each corresponding to a
// keyboard context: upper-alpha, lower-alpha, numeric and symbol.  the keypad
// is drawn within a "keyboard area" that is made of the kbd_buffer and the
// keypad.  the keypad is drawn as 4 rows of keys, with 5 special-keys present
// in every layout.  the 5 special-keys are: space-bar, back-space, enter,
// shift, and numeric.  the space-bar, backspace, and enter keys behave as
// typical without further comment.  the numeric key switches between the
// numeric and alpha-lower layouts.  the shift key shifts between alpha-upper
// and alpha-lower, or numeric and symbol layouts.
// as keys are tapped, the running content in kbd_buffer is displayed
// immediately above the keypad at the top of the "keyboard area."  when enter
// is pressed, the kbd_buffer is returned as a string to the caller of
// update_keyboard().  In this demonstration, as full lines of text are
// returned by update_keyboard(), they are printed onto the display area above
// the keyboard area.
//
// hardware test config:
//  ESP32-S3 DevKitC-1
//  MSP3218 TFT LCD 3.2-inch SPI Module ILI9341 (TFT)
//  a.k.a. 3.2 TFT SPI 240X320 "Red Display"
//  - integrated XPT2046 Resistive Touch Panel (TP)
//
//  this example uses the MSP "Red Display" and it can be easily refactored
//  to other models of display, other sizes and processors, such as the
//  "Cheap Yellow Display", ESP32, or STM32.
//
// software dependencies:
//  adafruit/Adafruit_GFX library refer to library notes
//  adafruit/Adafruit ILI9341 library and its dependencies
//  tedtoal/XPT2046_Touchscreen_TT library
//
//  this example uses the popular Adafruit GFX library, and it can be easily
//  refactored to other libraries, such as TFT-eSPI, Lovyan GFX, etc.  For
//  the sake of brevity, the draw_key() method uses GFX->fillRoundRect()
//  to draw the keys.
//
// library notes:
// - XPT2046 Touch Panel library: use of the Ted Toal library gives us
//  calibrated touch coordinates matching display pixel dimensions.

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <TS_Display.h>
#include "MSP3218_config_esp32.h"

// set the TFT_ROTATION so we can normalize the touch points to
// the TFT display coordinates and correctly orient the keyboard.
// typically, (0,2) are portrait, (1,3) are landscape orientation.
#define TFT_ROTATION 1

// display colors for demo
#define COLOR_TFTBG ILI9341_BLUE
#define COLOR_TFTFG ILI9341_WHITE

// the keyboard is normally drawn centered at the bottom of the
// TFT display orienation.  The position may be adjusted slightly
// using the KBD_TOP and KBD_LEFT parameters.
#define KBD_TOP 0  // px (move up using negative values)
#define KBD_LEFT 0 // px (<0 = left, >0 = right)

// keyboard area layout diagram
//  +-------------------------------+
//  | KBD_BUFFER_AREA_____________  |
//  | q  w	e  r  t  y  u  i  o  p  |
//  |   a  s  d  f  g  h  j  k  l	  |
//  | SH  z  x  c  v  b  n  m  BSP  |
//  | 12   | --SPACE BAR-- |   ENT  |
//  +-------------------------------+
// at the top of the "keyboard area" is the keyboard buffer area (kbd_buffer)
// where the keys typed display their content, prior to pressing enter (ENT).
// the "special keys" are shift (SH), numeric (12), space (SPACE BAR),
// backspace (BSP) and enter (ENT).  the special keys are present and fixed
// regardless the active keyboard layout.
// the "standard keys" are the majority of keys shown in lowercase such as
// "Q W E R T Y" and these are changed based upon the active keyboard
// context layout, such as upper-alpha, lower-alpha, numeric and symbol.

// keyboard layouts
#define KBD_IDX_LEFT 0 // index of key position from left of key row
#define KBD_IDX_MAX 1  // index of max key column index
#define KBD_IDX_DATA 2 // index of key name text
#define KBD_ROW_MAX 3  // maximum rows in keyboard layout
#define KBD_COL_MAX 12 // maximum cols in layout row
typedef const char kbd_layout_row_t[KBD_COL_MAX];
typedef kbd_layout_row_t kbd_layout_t[KBD_ROW_MAX];
kbd_layout_t Layout_Upper PROGMEM = {
    {00, 12, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'},
    {10, 11, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L'},
    {30, 9, 'Z', 'X', 'C', 'V', 'B', 'N', 'M'},
};
kbd_layout_t Layout_Lower PROGMEM = {
    {00, 12, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
    {10, 11, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'},
    {30, 9, 'z', 'x', 'c', 'v', 'b', 'n', 'm'},
};
kbd_layout_t Layout_Numeric PROGMEM = {
    {00, 12, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
    {00, 12, '-', '/', ':', ';', '(', ')', '$', '&', '@', '"'},
    {50, 7, '.', ',', '?', '!', '\''}};
kbd_layout_t Layout_Symbol PROGMEM = {
    {00, 12, '[', ']', '{', '}', '#', '%', '^', '*', '+', '='},
    {40, 8, '_', '\\', '|', '~', '<', '>'},
    {50, 7, '.', ',', '?', '!', '\''}};

// keyboard (internal) constants
#define KBD_WIDTH 240                      // keyboard area
#define KBD_HEIGHT 140                     // keyboard area
#define KBD_REPEAT_DELAY 250               // ms press delay before repeating
#define KBD_CLR_BG 0xCE59                  // color keyboard area
#define KBD_CLR_FG 0x0000                  // color buffer text
#define KBD_CLR_FILL 0x31A6                // color button fill
#define KBD_CLR_TEXT 0xFFFF                // color button text
#define KBD_CLR_DROP 0x8410                // color button drop shadow color
#define KBD_CLR_STROKE 0xFFFF              // color button border
#define KBD_TEXTSIZE 2                     // key face text size
#define KBD_CHARW (KBD_TEXTSIZE * 6)       // key face char width
#define KBD_CHARH (KBD_TEXTSIZE * 8)       // key face char height
#define KBD_LINEH (KBD_CHARH + 4)          // key face height
#if TFT_ROTATION == 0 || TFT_ROTATION == 2 // portrait
#define KBD_TOP2 (KBD_TOP + 180)           // keyboard position from top (px) (with buffer)
#define KBD_LEFT2 (KBD_LEFT)               // keyboard position from left (px)
#else                                      // landscape
#define KBD_TOP2 (KBD_TOP + 100)           // keyboard position from top (px) (with buffer)
#define KBD_LEFT2 (KBD_LEFT + 35)          // keyboard position from left (px)
#endif
#define KBD_POSY KBD_TOP2 + KBD_LINEH   // keyboard position from top (px) (after buffer)
#define KBD_KEY_W1 (1 * KBD_CHARW + 8)  // width standard key
#define KBD_KEY_W2 (2 * KBD_CHARW + 6)  // width shift/numeric key
#define KBD_KEY_W3 (3 * KBD_CHARW + 2)  // width bksp/enter key
#define KBD_KEY_W4 (11 * KBD_CHARW + 8) // width space bar
#define KBD_KEY_H (KBD_LINEH + 5)       // key height
#define KBD_ROW_H (KBD_LINEH + 10)      // key pad row height
#define KBD_X_SH (5 + KBD_LEFT2)
#define KBD_Y_SH (KBD_POSY + 2 * KBD_ROW_H)
#define KBD_X_NUM (5 + KBD_LEFT2)
#define KBD_Y_NUM (KBD_POSY + 3 * KBD_ROW_H)
#define KBD_X_SB (47 + KBD_LEFT2)
#define KBD_Y_SB KBD_Y_NUM
#define KBD_X_BSP (200 + KBD_LEFT2)
#define KBD_Y_BSP KBD_Y_SH
#define KBD_X_ENT (200 + KBD_LEFT2)
#define KBD_Y_ENT KBD_Y_NUM
#define KBD_X_STD(kcol, kleft) (8 + (23 * (kcol - KBD_IDX_DATA)) + kleft)
#define KBD_Y_STD(krow) (KBD_POSY + (KBD_ROW_H * krow))

#define KBD_BUFLEN 18 // floor(tft->width()/(KBD_CHARW+1))
char kbd_buffer[KBD_BUFLEN + 1] = {0};

Adafruit_ILI9341 *tft;
XPT2046_Touchscreen *ts;
TS_Display *ts_display;

bool setup_display()
{
  tft = new Adafruit_ILI9341(TFT_CS, TFT_DC);
  tft->begin();
  tft->setRotation(TFT_ROTATION);
  tft->setCursor(0, 0);
  tft->fillScreen(COLOR_TFTBG);
  return true;
}
bool setup_touch()
{
  ts = new XPT2046_Touchscreen(TS_CS);
  if (ts->begin())
  {
    ts->setRotation(TFT_ROTATION);
    ts_display = new TS_Display();
    ts_display->begin(ts, tft);
    ts_display->setTS_calibration(MY_TS_LR_X, MY_TS_LR_Y, MY_TS_UL_X, MY_TS_UL_Y);
    ts->setThresholds(MY_TS_THRESH);
    return true;
  }
  return false;
}
TS_Point getPoint()
{
  TS_Point p = ts->getPoint();
  ts_display->mapTStoDisplay(p.x, p.y, &p.x, &p.y);
  return p;
}

kbd_layout_row_t *get_keyboard(bool shift, bool numeric)
{
  return numeric
             ? (shift ? &Layout_Symbol[0] : &Layout_Numeric[0])
             : (shift ? &Layout_Upper[0] : &Layout_Lower[0]);
}
void draw_key(int x, int y, int w, int h)
{
  tft->fillRoundRect(x - 0, y + 1, w, h, 3, KBD_CLR_DROP);
  tft->fillRoundRect(x, y, w, h, 3, KBD_CLR_FILL);
}
void draw_keyboard(bool shift, bool numeric)
{
  // save cursor
  int curX = tft->getCursorX();
  int curY = tft->getCursorY();

  // clear keyboard area
  tft->fillRect(0 + KBD_LEFT2, KBD_TOP2, KBD_WIDTH, KBD_HEIGHT, KBD_CLR_BG);
  tft->setTextSize(KBD_TEXTSIZE);
  tft->setTextColor(KBD_CLR_TEXT, KBD_CLR_FILL);

  // select layout
  kbd_layout_row_t *layout = get_keyboard(shift, numeric);

  // standard buttons
  for (int krow = 0; krow < KBD_ROW_MAX; krow++)
  {
    int posLeft = pgm_read_byte(&(layout[krow][KBD_IDX_LEFT])) + KBD_LEFT2;

    for (int x = KBD_IDX_DATA; x < KBD_COL_MAX; x++)
    {
      if (x >= pgm_read_byte(&(layout[krow][KBD_IDX_MAX])))
        break;
      draw_key(KBD_X_STD(x, posLeft), KBD_Y_STD(krow), KBD_KEY_W1, KBD_KEY_H);
      tft->setCursor(4 + KBD_X_STD(x, posLeft), 5 + KBD_Y_STD(krow));
      tft->print(char(pgm_read_byte(&(layout[krow][x]))));
    }
  }
  // special buttons
  draw_key(KBD_X_SH, KBD_Y_SH, KBD_KEY_W2, KBD_KEY_H);
  tft->setCursor(numeric ? 4 + KBD_X_SH : 10 + KBD_X_SH, 5 + KBD_Y_SH);
  tft->print(numeric ? F("+=") : F("^"));

  draw_key(KBD_X_NUM, KBD_Y_NUM, KBD_KEY_W2, KBD_KEY_H);
  tft->setCursor(4 + KBD_X_NUM, 5 + KBD_Y_NUM);
  tft->print(numeric ? F("Aa") : F("12"));

  draw_key(KBD_X_BSP, KBD_Y_BSP, KBD_KEY_W3, KBD_KEY_H);
  tft->setCursor(3 + KBD_X_BSP, 5 + KBD_Y_BSP);
  tft->print(F("bsp"));

  draw_key(KBD_X_ENT, KBD_Y_ENT, KBD_KEY_W3, KBD_KEY_H);
  tft->setCursor(3 + KBD_X_ENT, 5 + KBD_Y_ENT);
  tft->print(F("ent"));

  draw_key(KBD_X_SB, KBD_Y_SB, KBD_KEY_W4, KBD_KEY_H);
  tft->setCursor(43 + KBD_X_SB, 5 + KBD_Y_SB);
  tft->print(F("space"));

  // restore cursor
  tft->setCursor(curX, curY);
}
bool key_pressed(int X, int Y, int x, int y, int w, int h)
{
  bool hit = (X > x && X < x + w && Y > y && Y < y + h);
  // tft->fillCircle(X, Y, 2, 0x0FF0);   // show touch point to debug
  return hit;
}
// returns TRUE when the "enter" key is pressed and the kbd_buffer is moved
// to *textReturn.
bool update_keyboard(char *textReturn)
{
  static int bufIndex = 0;
  static bool shift = false, numeric = false, last_shift = false, last_numeric = false;
  static TS_Point lastPoint = {};
  static unsigned long lastTime = 0;
  bool notRepeat = true;
  bool clear = false;  // clear text area on exit
  bool retval = false; // signal caller they have result string

  if (ts->touched())
  {
    TS_Point p = getPoint();

    // repeat (touch) blocker
    if (!key_pressed(p.x, p.y, KBD_X_BSP, KBD_Y_BSP, KBD_KEY_W3, KBD_KEY_H) && (lastPoint == p || millis() - lastTime < KBD_REPEAT_DELAY))
    {
      // when it's not the backspace key, and the point hasn't changed
      // or the timeout hasn't elapsed, we have a repeat to ignore.
      notRepeat = false;
    }
    else
    {
      lastPoint = p;
      lastTime = millis();
    }
    // check keys for pressed
    if (/* repeat ok */ key_pressed(p.x, p.y, KBD_X_BSP, KBD_Y_BSP, KBD_KEY_W3, KBD_KEY_H))
    {
      // backspace
      if ((bufIndex) > 0)
        bufIndex--;
      kbd_buffer[bufIndex] = 0;
      clear = true;
      delay(200); // repeat rate
    }
    else if (notRepeat && key_pressed(p.x, p.y, KBD_X_SH, KBD_Y_SH, KBD_KEY_W2, KBD_KEY_H))
    {
      // shift key
      shift = !shift;
    }
    else if (notRepeat && key_pressed(p.x, p.y, KBD_X_NUM, KBD_Y_NUM, KBD_KEY_W2, KBD_KEY_H))
    {
      // numeric key
      numeric = !numeric;
      last_shift = shift = false; // unset shift on numeric key
    }
    else if (notRepeat && key_pressed(p.x, p.y, KBD_X_SB, KBD_Y_SB, KBD_KEY_W4, KBD_KEY_H))
    {
      // space bar
      if (bufIndex < (KBD_BUFLEN - 1))
      {
        kbd_buffer[bufIndex++] = ' ';
        kbd_buffer[bufIndex] = 0;
      }
    }
    else if (notRepeat && key_pressed(p.x, p.y, KBD_X_ENT, KBD_Y_ENT, KBD_KEY_W3, KBD_KEY_H))
    {
      // enter key
      strcpy(textReturn, kbd_buffer);
      memset(kbd_buffer, 0, sizeof kbd_buffer);
      bufIndex = 0;
      retval = clear = true;
      numeric = shift = false;
    }
    else if (notRepeat)
    {
      // standard keys
      bool keySearch = true; // test keys for pressed until we find one
      char keyText[2] = {0, 0};
      kbd_layout_row_t *layout = get_keyboard(shift, numeric);
      for (int y = 0; keySearch && y < KBD_ROW_MAX; y++)
      {
        int posLeft = pgm_read_byte(&(layout[y][KBD_IDX_LEFT])) + KBD_LEFT2;
        int keyMax = pgm_read_byte(&(layout[y][KBD_IDX_MAX]));
        for (int x = KBD_IDX_DATA; x < KBD_COL_MAX && x < keyMax; x++)
        {
          keyText[0] = pgm_read_byte(&(layout[y][x]));
          if (key_pressed(p.x, p.y, KBD_X_STD(x, posLeft), KBD_Y_STD(y), KBD_KEY_W1, KBD_KEY_H))
          {
            if (bufIndex < (KBD_BUFLEN - 1))
            {
              kbd_buffer[bufIndex++] = keyText[0];
              kbd_buffer[bufIndex] = 0;
            }
            keySearch = false; // break from key search
            break;
          }
        }
      }
    }

    if (numeric != last_numeric || shift != last_shift)
    {
      // layout change
      draw_keyboard(shift, numeric);
      last_shift = shift;
      last_numeric = numeric;
    }
  }
  if (notRepeat)
  {
    // update text area
    tft->setTextColor(KBD_CLR_FG, KBD_CLR_BG);
    int curX = tft->getCursorX();
    int curY = tft->getCursorY();
    {
      if (clear)
      {
        tft->setCursor(15 + KBD_LEFT2, KBD_TOP2 + 1);
        tft->print(F("                 "));
      }
      tft->setCursor(15 + KBD_LEFT2, KBD_TOP2 + 1);
      tft->print(kbd_buffer);
    }
    tft->setCursor(curX, curY);
  }
  return retval;
}

void setup(void)
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  setup_display();
  tft->println(F("display started"));

  if (!setup_touch())
  {
    tft->println(F("touch failure"));
    while (1)
      ;
  }
  tft->println(F("touch started"));

  tft->fillScreen(COLOR_TFTBG);
  tft->setTextColor(COLOR_TFTFG, COLOR_TFTBG);
  draw_keyboard(false, false);
}

void loop()
{
  char myText[KBD_BUFLEN] = {0};
  if (update_keyboard(myText) && strlen(myText))
  {
    tft->setTextColor(COLOR_TFTFG, COLOR_TFTBG);
    tft->println(myText);
  }
}
