# Simple Touch Keyboard
A simple touch keyboard designed for low-resolution displays.  Ideal for small TFT displays (240×320) and AVR MCUs (Arduino Uno R3). 
Optimized for ILI9341 display + XPT2046 touch panel. 
Supports multiple layouts, repeating backspace, and screen rotation.

## Overview

This project provides a lightweight, easy-to-use touch keyboard implementation optimized for devices with limited screen real estate. It's particularly well-suited for embedded systems, IoT devices, and small displays where traditional on-screen keyboards may be too complex or resource-intensive.

## Features

-   **Low-resolution optimized**: Designed specifically for displays with limited pixel density
-   **Lightweight**: Minimal resource footprint
-   **Simple implementation**: Easy to integrate into existing projects
-   **Touch-friendly**: Optimized for touch input interfaces

## Detail
For projects in need of alpha-numeric user input via touch panel on smaller lightweight MCUs, this project demonstrates a fully functioning keyboard similar to those presented by mobile devices, and offers a means to get user input to their application. 

For the majority of MCU-controlled touch displays, the [LvGL](https://lvgl.io/) Library is a common solution to present graphical user interfaces and, where needed, the alpha-numeric touch keyboard. But the memory and speed requirements for LvGL exceed those of the more lightweight MCU, such as the Arduino Uno R3, with its limited speed and memory.  Smaller MCUs are limited in the display resolution they can realistically operate.  

Using smaller color displays, such as the 240x320 TFT LCD supported by the ILI9341 and ST7789 display driver ICs, the limited resolution can provide challenges presenting the fully functional alpha-numeric keyboard. 

As with any software solution, "there is more than one way to skin a cat." This project simply presents one method to resolve the issue, and may help the consumer know what is possible, and give them a head-start when in need of user keyboard input.

## Theory of Operation
Presented here are 4 keypad (keyboard) layouts, each corresponding to a keyboard context: upper-alpha, lower-alpha, numeric and symbol.

| upper-alpha | lower-alpha |
| :---: | :---: |
| ![upper-alpha](assets/Layout_Upper.png) | ![upper-lower](assets/Layout_Lower.png) |
| numeric | symbol |
| ![numeric](assets/Layout_Numeric.png) | ![symbol ](assets/Layout_Symbol.png) |

The keypad is drawn within a "keyboard area" that is made of the kbd_buffer and the keypad.  

![typing](assets/typing.png)

The keypad is drawn as 4 rows of keys, with 5 special-keys present in every layout.  the 5 special-keys are: space-bar, back-space, enter, shift, and numeric.  The space-bar, backspace, and enter keys behave as typical without further comment.  The numeric key switches between the numeric and alpha-lower layouts.  The shift key shifts between alpha-upper and alpha-lower, or numeric and symbol layouts. 

![entered](assets/entered.png)

As keys are tapped, the running content in `kbd_buffer` is displayed immediately above the keypad at the top of the "keyboard area."  when enter is pressed, the kbd_buffer is returned as a string to the caller of `update_keyboard()`.  In this demonstration, as full lines of text are returned by update_keyboard(), they are printed onto the display area above the keyboard area. 

## Usage

Once the hardware display and touch are initialized in `setup()`, simply call `draw_keyboard()` to paint the keyboard to the display.

In your `loop()` function, call `update_keyboard()` at regular intervals to process user input and handle the resulting strings as desired.
```cpp
void setup(void)
{
	Serial.begin(115200);
	while (!Serial)
	delay(10);
 
	setup_display();
	setup_touch();
	draw_keyboard(false, false);
}

void loop()
{
	char  myText[KBD_BUFLEN] = {0};
	if (update_keyboard(myText) && strlen(myText))
	{
		tft->println(myText);
	}
}
```

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](https://claude.ai/chat/LICENSE) file for details.
