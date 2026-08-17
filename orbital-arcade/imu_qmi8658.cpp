#include "imu_qmi8658.h"
#include <math.h>

IMU_QMI8658 IMU;

IMU_QMI8658::IMU_QMI8658() : 
    _gravityAngle(0), 
    _filteredAngle(0),
    _currentQuadrant(0),
    _lastRotateTime(0),
    _initialized(false) {}

bool IMU_QMI8658::begin(TwoWire &wire, uint8_t addr) {
    if (_qmi.begin(wire, addr, 15, 14)) {
        _qmi.configAccelerometer(SensorQMI8658::ACC_RANGE_4G, SensorQMI8658::ACC_ODR_1000Hz, SensorQMI8658::LPF_MODE_0);
        _qmi.enableAccelerometer();
        _initialized = true;
        return true;
    }
    if (_qmi.begin(wire, QMI8658_H_SLAVE_ADDRESS, 15, 14)) {
        _qmi.configAccelerometer(SensorQMI8658::ACC_RANGE_4G, SensorQMI8658::ACC_ODR_1000Hz, SensorQMI8658::LPF_MODE_0);
        _qmi.enableAccelerometer();
        _initialized = true;
        return true;
    }
    return false;
}

void IMU_QMI8658::resetTiltState() {
    _currentQuadrant = 0;
    _lastRotateTime = millis();
}

void IMU_QMI8658::update() {
    if (!_initialized) return;

    if (_qmi.getAccelerometer(_acc.x, _acc.y, _acc.z)) {
        // Calculate 2D gravity angle in degrees (-180 to +180) across the screen plane
        float rawDeg = atan2f(_acc.y, _acc.x) * (180.0f / M_PI);
        _gravityAngle = rawDeg;

        // Map gravity angle directly to the 4 physical well orientations:
        if (_gravityAngle >= -45.0f && _gravityAngle < 45.0f) {
            _currentQuadrant = 0; // Bottom Well (GRAVITY_DOWN)
        } else if (_gravityAngle >= 45.0f && _gravityAngle < 135.0f) {
            _currentQuadrant = 1; // Left Well (GRAVITY_LEFT)
        } else if (_gravityAngle >= 135.0f || _gravityAngle < -135.0f) {
            _currentQuadrant = 2; // Top Well (GRAVITY_UP)
        } else if (_gravityAngle >= -135.0f && _gravityAngle < -45.0f) {
            _currentQuadrant = 3; // Right Well (GRAVITY_RIGHT)
        }
    }
}

float IMU_QMI8658::getAngle() {
    return _gravityAngle;
}

IMURotationEvent IMU_QMI8658::checkTiltRotation(float thresholdDeg) {
    update();
    return IMU_ROTATE_NONE;
}
