#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

struct AppConfig {
    uint8_t brightness;
    uint16_t timeoutSec;
    bool gyroEnabled;
    float tiltSensitivity;
    uint32_t highScore;       // Tetris High Score
    uint32_t snakeHighScore;  // Snake High Score
};

class SDConfigManager {
public:
    SDConfigManager();
    bool begin();
    void loadConfig();
    void saveHighScore(uint32_t score);
    void saveSnakeHighScore(uint32_t score);
    const AppConfig& getConfig() const { return _config; }

private:
    AppConfig _config;
    Preferences _prefs;
};

extern SDConfigManager SDConfig;
