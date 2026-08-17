#include "touch.h"

TouchDriver Touch;

static volatile bool touchIntFlag = false;

static void IRAM_ATTR touchISR() {
    touchIntFlag = true;
}

TouchDriver::TouchDriver() : 
    _wire(&Wire), 
    _addr(0x15), 
    _isFT3168(false), 
    _isDragging(false), 
    _startX(0), 
    _startY(0), 
    _currentX(0), 
    _currentY(0), 
    _startTime(0), 
    _hasMoved(false),
    _ignoreNextRelease(false) {}

void TouchDriver::reset() {
    _isDragging = false;
    _startX = 0;
    _startY = 0;
    _currentX = 0;
    _currentY = 0;
    _startTime = 0;
    _hasMoved = false;
    _ignoreNextRelease = true;
    touchIntFlag = false;
}

bool TouchDriver::begin(TwoWire &wire) {
    _wire = &wire;

    pinMode(TOUCH_INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TOUCH_INT_PIN), touchISR, FALLING);

    // CST820 (V2 @ 0x15)
    _wire->beginTransmission(0x15);
    if (_wire->endTransmission() == 0) {
        _addr = 0x15;
        _isFT3168 = false;

        _wire->beginTransmission(0x15);
        _wire->write(0xFA);
        _wire->write(0b00010000);
        _wire->endTransmission();
        delay(20);

        return true;
    }

    // FT3168 (V1 @ 0x38)
    _wire->beginTransmission(0x38);
    if (_wire->endTransmission() == 0) {
        _addr = 0x38;
        _isFT3168 = true;
        return true;
    }

    _addr = 0x15;
    _isFT3168 = false;
    return false;
}

bool TouchDriver::read(TouchPoint &point) {
    _wire->beginTransmission(_addr);
    _wire->write(0x00);
    if (_wire->endTransmission() != 0) {
        point.isPressed = false;
        return false;
    }

    _wire->requestFrom(_addr, (uint8_t)7);
    if (_wire->available() < 7) {
        point.isPressed = false;
        return false;
    }

    uint8_t buf[7];
    for (int i = 0; i < 7; i++) {
        buf[i] = _wire->read();
    }

    // Byte 2: Finger points count (0 = no touch, 1 = 1 point)
    uint8_t numPoints = buf[2] & 0x0F;
    if (numPoints == 0 || numPoints > 2) {
        point.isPressed = false;
        return false;
    }

    int16_t rawX = ((int16_t)(buf[3] & 0x0F) << 8) | buf[4];
    int16_t rawY = ((int16_t)(buf[5] & 0x0F) << 8) | buf[6];

    point.x = constrain(rawX, 0, SCREEN_WIDTH - 1);
    point.y = constrain(rawY, 0, SCREEN_HEIGHT - 1);
    point.isPressed = true;
    return true;
}

TouchGesture TouchDriver::processGestures(int16_t &deltaX, int16_t &deltaY) {
    TouchPoint pt;
    bool pressed = read(pt);
    uint32_t now = millis();
    TouchGesture gesture = GESTURE_NONE;
    deltaX = 0;
    deltaY = 0;

    if (pressed) {
        if (!_isDragging) {
            _isDragging = true;
            _startX = pt.x;
            _startY = pt.y;
            _currentX = pt.x;
            _currentY = pt.y;
            _startTime = now;
            _hasMoved = false;
            gesture = GESTURE_START_GAME;
        } else {
            _currentX = pt.x;
            _currentY = pt.y;

            int16_t dx = _currentX - _startX;
            int16_t dy = _currentY - _startY;

            // Step threshold: 22px (matched to 26px blocks)
            if (abs(dx) >= 22) {
                deltaX = (dx > 0) ? 1 : -1;
                _startX = _currentX;
                _hasMoved = true;
            }

            if (abs(dy) >= 22) {
                deltaY = (dy > 0) ? 1 : -1;
                _startY = _currentY;
                _hasMoved = true;
            }
        }
    } else {
        if (_isDragging) {
            _isDragging = false;
            _ignoreNextRelease = false;
        }
    }

    return gesture;
}
