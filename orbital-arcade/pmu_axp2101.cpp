#include "pmu_axp2101.h"

PMU_AXP2101 PMU;

PMU_AXP2101::PMU_AXP2101() : _isOnline(false) {}

bool PMU_AXP2101::begin(TwoWire &wire) {
    if (!_pmu.begin(wire, AXP2101_SLAVE_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN)) {
        return false;
    }
    _isOnline = true;

    // Enable and set ALDO power rails (Logic, IO, Sensors, Touch)
    _pmu.setALDO1Voltage(3300);
    _pmu.enableALDO1();

    _pmu.setALDO2Voltage(3300);
    _pmu.enableALDO2();

    _pmu.setALDO3Voltage(3300);
    _pmu.enableALDO3();

    _pmu.setALDO4Voltage(3300);
    _pmu.enableALDO4();

    // Enable and set BLDO power rails (AMOLED Panel Power)
    _pmu.setBLDO1Voltage(3300);
    _pmu.enableBLDO1();

    _pmu.setBLDO2Voltage(3300);
    _pmu.enableBLDO2();

    // Enable and set DLDO power rails
    _pmu.setDLDO1Voltage(3300);
    _pmu.enableDLDO1();

    _pmu.setDLDO2Voltage(3300);
    _pmu.enableDLDO2();

    // Enable Battery ADC measurement
    _pmu.enableBattDetection();
    _pmu.enableVbusVoltageMeasure();
    _pmu.enableBattVoltageMeasure();
    _pmu.enableSystemVoltageMeasure();

    return true;
}

uint8_t PMU_AXP2101::getBatteryPercent() {
    if (!_isOnline) return 100;
    return _pmu.getBatteryPercent();
}

bool PMU_AXP2101::isCharging() {
    if (!_isOnline) return false;
    return _pmu.isCharging();
}

bool PMU_AXP2101::isBatteryConnected() {
    if (!_isOnline) return false;
    return _pmu.isBatteryConnect();
}

float PMU_AXP2101::getBatteryVoltage() {
    if (!_isOnline) return 4.2f;
    return (float)_pmu.getBattVoltage() / 1000.0f;
}

void PMU_AXP2101::powerOff() {
    if (_isOnline) {
        _pmu.shutdown();
    }
}
