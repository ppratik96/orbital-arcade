#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "config.h"

class DisplayAMOLED {
public:
    DisplayAMOLED();
    bool begin();
    void setBrightness(uint8_t brightness);
    void fillScreen(uint16_t color);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    
    void drawChar(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg, uint8_t size = 1);
    void drawString(int16_t x, int16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size = 1);
    void drawCenteredString(int16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size = 1);
    
    void drawBeveledBlock(int16_t x, int16_t y, int16_t size, uint16_t color, bool isGhost = false);

private:
    Arduino_DataBus *_bus;
    Arduino_GFX *_gfx;
};

extern DisplayAMOLED Display;
