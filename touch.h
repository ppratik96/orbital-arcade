#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

enum TouchGesture {
    GESTURE_NONE = 0,
    GESTURE_TAP_LEFT,
    GESTURE_TAP_RIGHT,
    GESTURE_START_GAME
};

struct TouchPoint {
    int16_t x;
    int16_t y;
    bool isPressed;
};

class TouchDriver {
public:
    TouchDriver();
    bool begin(TwoWire &wire = Wire);
    bool read(TouchPoint &point);
    TouchGesture processGestures(int16_t &deltaX, int16_t &deltaY);
    void reset();

private:
    TwoWire *_wire;
    uint8_t _addr;
    bool _isFT3168;

    bool _isDragging;
    int16_t _startX;
    int16_t _startY;
    int16_t _currentX;
    int16_t _currentY;
    uint32_t _startTime;
    bool _hasMoved;
    bool _ignoreNextRelease;
};

extern TouchDriver Touch;
