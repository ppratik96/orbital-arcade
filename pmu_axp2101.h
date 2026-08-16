#pragma once

#define XPOWERS_CHIP_AXP2101

#include <Arduino.h>
#include <Wire.h>
#include "XPowersLib.h"
#include "config.h"

class PMU_AXP2101 {
public:
    PMU_AXP2101();
    bool begin(TwoWire &wire = Wire);
    uint8_t getBatteryPercent();
    bool isCharging();
    bool isBatteryConnected();
    float getBatteryVoltage();
    void powerOff();

private:
    XPowersPMU _pmu;
    bool _isOnline;
};

extern PMU_AXP2101 PMU;
