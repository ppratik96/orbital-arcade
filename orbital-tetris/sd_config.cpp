#include "sd_config.h"

SDConfigManager SDConfig;

SDConfigManager::SDConfigManager() {
    _config.brightness = 200;
    _config.timeoutSec = 120;
    _config.gyroEnabled = true; // Gyroscope is the PRIMARY mechanic
    _config.tiltSensitivity = 35.0f;
    _config.highScore = 0;
    _config.snakeHighScore = 0;
}

bool SDConfigManager::begin() {
    loadConfig();
    return true;
}

void SDConfigManager::loadConfig() {
    _prefs.begin("orbital_tetris", false);
    _config.highScore = _prefs.getUInt("high_score", 0);
    _config.snakeHighScore = _prefs.getUInt("snake_hi", 0);
    _config.brightness = _prefs.getUChar("brightness", 200);
    _config.timeoutSec = _prefs.getUShort("timeout", 120);
    _config.gyroEnabled = _prefs.getBool("gyro", true);
    _config.tiltSensitivity = _prefs.getFloat("tilt_sens", 35.0f);
    _prefs.end();
}

void SDConfigManager::saveHighScore(uint32_t score) {
    if (score <= _config.highScore) return;
    _config.highScore = score;

    _prefs.begin("orbital_tetris", false);
    _prefs.putUInt("high_score", score);
    _prefs.end();
}

void SDConfigManager::saveSnakeHighScore(uint32_t score) {
    if (score <= _config.snakeHighScore) return;
    _config.snakeHighScore = score;

    _prefs.begin("orbital_tetris", false);
    _prefs.putUInt("snake_hi", score);
    _prefs.end();
}
