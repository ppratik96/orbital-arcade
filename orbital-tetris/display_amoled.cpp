#include "display_amoled.h"
#include <Wire.h>

DisplayAMOLED Display;

// TCA9554 IO Expander Registers (0x20)
#define TCA9554_INPUT_REG   0x00
#define TCA9554_OUTPUT_REG  0x01
#define TCA9554_CONFIG_REG  0x03

static void initIOExpander() {
    Wire.beginTransmission(IO_EXPANDER_ADDR);
    Wire.write(TCA9554_CONFIG_REG);
    Wire.write(0b10111000); // Pins 0, 1, 2, 6 output (bits 0,1,2,6 = 0)
    Wire.endTransmission();

    Wire.beginTransmission(IO_EXPANDER_ADDR);
    Wire.write(TCA9554_OUTPUT_REG);
    Wire.write(0b00000000); // Pins 0, 1, 2, 6 LOW
    Wire.endTransmission();
    delay(30);

    Wire.beginTransmission(IO_EXPANDER_ADDR);
    Wire.write(TCA9554_OUTPUT_REG);
    Wire.write(0b01000111); // Pins 0, 1, 2, 6 HIGH (bits 0,1,2,6 = 1)
    Wire.endTransmission();
    delay(60);
}

DisplayAMOLED::DisplayAMOLED() : _bus(nullptr), _gfx(nullptr) {}

bool DisplayAMOLED::begin() {
    initIOExpander();

    _bus = new Arduino_ESP32QSPI(
        LCD_CS_PIN,   /* CS = 12 */
        LCD_SCK_PIN,  /* SCK = 11 */
        LCD_D0_PIN,   /* D0 = 4 */
        LCD_D1_PIN,   /* D1 = 5 */
        LCD_D2_PIN,   /* D2 = 6 */
        LCD_D3_PIN    /* D3 = 7 */
    );

    _gfx = new Arduino_CO5300(
        _bus,
        GFX_NOT_DEFINED, /* RST = -1 (managed by IO Expander) */
        0,               /* rotation */
        SCREEN_WIDTH,    /* width = 368 */
        SCREEN_HEIGHT,   /* height = 448 */
        16,              /* col_offset1 = 16 */
        0,               /* row_offset1 = 0 */
        0,               /* col_offset2 = 0 */
        0                /* row_offset2 = 0 */
    );

    if (!_gfx->begin()) {
        delete _gfx;
        _gfx = new Arduino_SH8601(
            _bus,
            GFX_NOT_DEFINED,
            0,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            16,
            0,
            0,
            0
        );
        if (!_gfx->begin()) {
            return false;
        }
    }

    setBrightness(220);
    fillScreen(0x0000);
    return true;
}

void DisplayAMOLED::setBrightness(uint8_t brightness) {
    if (_bus) {
        _bus->beginWrite();
        _bus->writeC8D8(0x51, brightness);
        _bus->endWrite();
    }
}

void DisplayAMOLED::fillScreen(uint16_t color) {
    if (_gfx) _gfx->fillScreen(color);
}

void DisplayAMOLED::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (_gfx) _gfx->drawPixel(x, y, color);
}

void DisplayAMOLED::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (_gfx) _gfx->fillRect(x, y, w, h, color);
}

void DisplayAMOLED::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (_gfx) {
        _gfx->fillRect(x, y, w, 1, color);
        _gfx->fillRect(x, y + h - 1, w, 1, color);
        _gfx->fillRect(x, y, 1, h, color);
        _gfx->fillRect(x + w - 1, y, 1, h, color);
    }
}

void DisplayAMOLED::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (_gfx) _gfx->fillRect(x, y, w, 1, color);
}

void DisplayAMOLED::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    if (_gfx) _gfx->fillRect(x, y, 1, h, color);
}

void DisplayAMOLED::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (x0 == x1) {
        int16_t minY = min(y0, y1);
        int16_t h = abs(y1 - y0) + 1;
        drawFastVLine(x0, minY, h, color);
    } else if (y0 == y1) {
        int16_t minX = min(x0, x1);
        int16_t w = abs(x1 - x0) + 1;
        drawFastHLine(minX, y0, w, color);
    } else if (_gfx) {
        _gfx->drawLine(x0, y0, x1, y1, color);
    }
}

void DisplayAMOLED::drawChar(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg, uint8_t size) {
    if (_gfx) {
        _gfx->setCursor(x, y);
        _gfx->setTextColor(color, bg);
        _gfx->setTextSize(size);
        _gfx->print(c);
    }
}

void DisplayAMOLED::drawString(int16_t x, int16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size) {
    if (_gfx) {
        _gfx->setCursor(x, y);
        _gfx->setTextColor(color, bg);
        _gfx->setTextSize(size);
        _gfx->print(str);
    }
}

void DisplayAMOLED::drawCenteredString(int16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size) {
    int len = strlen(str);
    int16_t totalWidth = len * 6 * size;
    int16_t startX = (SCREEN_WIDTH - totalWidth) / 2;
    if (startX < 0) startX = 0;
    drawString(startX, y, str, color, bg, size);
}

void DisplayAMOLED::drawBeveledBlock(int16_t x, int16_t y, int16_t size, uint16_t color, bool isGhost) {
    if (!_gfx) return;

    if (isGhost) {
        _gfx->fillRect(x + 1, y + 1, size - 2, size - 2, COLOR_GHOST_FILL);
        drawRect(x, y, size, size, COLOR_GHOST_EDGE);
        return;
    }

    // 1. Outer 1px black border
    drawRect(x, y, size, size, 0x0000);

    // 2. Base block fill
    _gfx->fillRect(x + 1, y + 1, size - 2, size - 2, color);

    // 3. Highlight on top and left
    uint16_t highlight = 0xFFFF;
    _gfx->fillRect(x + 1, y + 1, size - 2, 1, highlight);
    _gfx->fillRect(x + 1, y + 1, 1, size - 2, highlight);

    // 4. Shadow on bottom and right
    uint16_t shadow = 0x0000;
    _gfx->fillRect(x + 1, y + size - 2, size - 2, 1, shadow);
    _gfx->fillRect(x + size - 2, y + 1, 1, size - 2, shadow);
}
