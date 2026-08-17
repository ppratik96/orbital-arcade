#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "SensorQMI8658.hpp"

enum IMURotationEvent {
    IMU_ROTATE_NONE = 0,
    IMU_ROTATE_CW,
    IMU_ROTATE_CCW
};

class IMU_QMI8658 {
public:
    IMU_QMI8658();
    bool begin(TwoWire &wire = Wire, uint8_t addr = QMI8658_L_SLAVE_ADDRESS);
    void update();
    float getAngle();
    float getAccX() const { return _acc.x; }
    float getAccY() const { return _acc.y; }
    float getAccZ() const { return _acc.z; }
    int8_t getCurrentQuadrant() const { return _currentQuadrant; }
    IMURotationEvent checkTiltRotation(float thresholdDeg = 0);
    void resetTiltState();
    bool isConnected() const { return _initialized; }

private:
    SensorQMI8658 _qmi;
    IMUdata _acc;
    IMUdata _gyr;
    float _gravityAngle;      // Continuous 0..360 degrees
    float _filteredAngle;
    int8_t _currentQuadrant;  // 0: Bottom, 1: Left, 2: Top, 3: Right
    uint32_t _lastRotateTime;
    bool _initialized;
};

extern IMU_QMI8658 IMU;
