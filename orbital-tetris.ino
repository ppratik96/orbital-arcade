#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "pmu_axp2101.h"
#include "display_amoled.h"
#include "touch.h"
#include "imu_qmi8658.h"
#include "sd_config.h"
#include "game_engine.h"
#include "snake_engine.h"

enum AppMode {
    MODE_ARCADE_HUB = 0,
    MODE_TETRIS,
    MODE_SNAKE
};

static AppMode currentMode = MODE_ARCADE_HUB;
static uint32_t lastFrameTime = 0;
static bool prevTouchPressed = false;

void drawArcadeHub() {
    Display.fillScreen(COLOR_BG);

    // Outer Cyber Frame
    Display.drawRect(10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20, COLOR_TEXT_CYAN);
    Display.drawRect(12, 12, SCREEN_WIDTH - 24, SCREEN_HEIGHT - 24, COLOR_TEXT_CYAN);

    // Title
    Display.drawCenteredString(25, "ORBITAL ARCADE", COLOR_TEXT_CYAN, COLOR_BG, 3);
    Display.drawCenteredString(55, "SELECT MISSION", COLOR_TEXT_GRAY, COLOR_BG, 1);

    // Card 1: ROTATETRIS (Y = 80..220, H = 140)
    Display.fillRect(25, 80, SCREEN_WIDTH - 50, 140, COLOR_WALL);
    Display.drawRect(25, 80, SCREEN_WIDTH - 50, 140, COLOR_TEXT_CYAN);
    Display.drawRect(26, 81, SCREEN_WIDTH - 52, 138, COLOR_TEXT_CYAN);
    Display.drawCenteredString(100, "ROTATETRIS", COLOR_TEXT_CYAN, COLOR_WALL, 2);
    Display.drawCenteredString(130, "4-Way Gravity Stacker", COLOR_TEXT_GRAY, COLOR_WALL, 1);
    char tetrisHi[32];
    snprintf(tetrisHi, sizeof(tetrisHi), "BEST: %lu", SDConfig.getConfig().highScore);
    Display.drawCenteredString(160, tetrisHi, COLOR_TEXT_WHITE, COLOR_WALL, 2);

    // Card 2: ROSNAKE (Y = 235..375, H = 140)
    Display.fillRect(25, 235, SCREEN_WIDTH - 50, 140, 0x01E0);
    Display.drawRect(25, 235, SCREEN_WIDTH - 50, 140, COLOR_TEXT_GREEN);
    Display.drawRect(26, 236, SCREEN_WIDTH - 52, 138, COLOR_TEXT_GREEN);
    Display.drawCenteredString(255, "ROSNAKE", COLOR_TEXT_GREEN, 0x01E0, 2);
    Display.drawCenteredString(285, "Gyro Tilt-Steered", COLOR_TEXT_GRAY, 0x01E0, 1);
    char snakeHi[32];
    snprintf(snakeHi, sizeof(snakeHi), "BEST: %lu", SDConfig.getConfig().snakeHighScore);
    Display.drawCenteredString(315, snakeHi, COLOR_TEXT_WHITE, 0x01E0, 2);

    // Footer Hint
    Display.drawCenteredString(400, "TAP CARD TO LAUNCH", COLOR_TEXT_GRAY, COLOR_BG, 1);
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n--- Orbital Arcade Booting ---");

    // 1. Initialize I2C Bus & PMU
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ);
    PMU.begin();

    // 2. Initialize AMOLED Display
    Display.begin();

    // 3. Initialize Touch Controller
    Touch.begin();

    // 4. Initialize IMU (QMI8658)
    IMU.begin();

    // 5. Initialize Preferences / High Scores
    SDConfig.begin();

    // 6. Initialize Game Engines
    Game.init();
    Snake.init();

    // 7. Show Arcade Hub Menu
    currentMode = MODE_ARCADE_HUB;
    drawArcadeHub();

    lastFrameTime = millis();
    Serial.println("--- Setup Completed ---");
}

void handleInputs() {
    int16_t deltaX = 0, deltaY = 0;
    TouchPoint pt;
    bool isPressed = Touch.read(pt);
    bool justTapped = (isPressed && !prevTouchPressed);
    prevTouchPressed = isPressed;

    // -------------------------------------------------------------
    // Physical BOOT Button
    // -------------------------------------------------------------
    if (digitalRead(BTN_BOOT_PIN) == LOW) {
        delay(50);
        if (digitalRead(BTN_BOOT_PIN) == LOW) {
            if (currentMode == MODE_TETRIS) {
                if (Game.getState() == STATE_PLAYING) {
                    Game.hardDrop();
                } else if (Game.getState() == STATE_GAME_OVER) {
                    Game.showStartScreen();
                } else if (Game.getState() == STATE_START_MENU) {
                    Game.resetGame();
                }
            } else if (currentMode == MODE_SNAKE) {
                if (Snake.getState() == SNAKE_STATE_GAME_OVER) {
                    Snake.showStartScreen();
                } else if (Snake.getState() == SNAKE_STATE_MENU) {
                    Snake.resetGame();
                }
            } else if (currentMode == MODE_ARCADE_HUB) {
                currentMode = MODE_TETRIS;
                Game.showStartScreen();
            }
            while (digitalRead(BTN_BOOT_PIN) == LOW) delay(10);
            return;
        }
    }

    // -------------------------------------------------------------
    // Arcade Hub Mode Touch
    // -------------------------------------------------------------
    if (currentMode == MODE_ARCADE_HUB) {
        if (justTapped) {
            if (pt.y >= 70 && pt.y <= 225) {
                currentMode = MODE_TETRIS;
                Game.showStartScreen();
                Touch.reset();
                prevTouchPressed = true; // Consume tap so start screen doesn't get triggered
                return;
            } else if (pt.y >= 230 && pt.y <= 385) {
                currentMode = MODE_SNAKE;
                Snake.showStartScreen();
                Touch.reset();
                prevTouchPressed = true; // Consume tap so start screen doesn't get triggered
                return;
            }
        }
        return;
    }

    // -------------------------------------------------------------
    // Rotatetris Mode Inputs
    // -------------------------------------------------------------
    if (currentMode == MODE_TETRIS) {
        if (Game.getState() == STATE_GAME_OVER) {
            if (justTapped) {
                if (pt.y >= 290 && pt.y <= 355) { // Play Again
                    Game.resetGame();
                } else if (pt.y >= 360) { // Main Menu
                    currentMode = MODE_ARCADE_HUB;
                    drawArcadeHub();
                }
                Touch.reset();
                prevTouchPressed = true;
                return;
            }
        } else if (Game.getState() == STATE_START_MENU) {
            if (justTapped) {
                Game.handleMenuTouch(pt.x, pt.y);
                Touch.reset();
                prevTouchPressed = true;
                return;
            }
        } else if (Game.getState() == STATE_PLAYING) {
            IMU.update();
            int8_t quadrant = IMU.getCurrentQuadrant();
            Game.setGravityDirection((GravityDirection)quadrant);

            Touch.processGestures(deltaX, deltaY);
            GravityDirection gDir = Game.getGravityDirection();
            int16_t lateral = 0;
            int16_t drop = 0;

            switch (gDir) {
                case GRAVITY_DOWN:
                    lateral = deltaX;
                    drop = (deltaY > 0) ? 1 : 0;
                    break;
                case GRAVITY_LEFT:
                    lateral = deltaY;
                    drop = (deltaX < 0) ? 1 : 0;
                    break;
                case GRAVITY_UP:
                    lateral = -deltaX;
                    drop = (deltaY < 0) ? 1 : 0;
                    break;
                case GRAVITY_RIGHT:
                    lateral = -deltaY;
                    drop = (deltaX > 0) ? 1 : 0;
                    break;
            }

            if (lateral != 0) Game.movePiece(lateral);
            if (drop > 0) Game.dropPiece();
        }
        return;
    }

    // -------------------------------------------------------------
    // RoSnake Mode Inputs
    // -------------------------------------------------------------
    if (currentMode == MODE_SNAKE) {
        if (Snake.getState() == SNAKE_STATE_GAME_OVER) {
            if (justTapped) {
                if (pt.y >= 290 && pt.y <= 355) { // Play Again
                    Snake.resetGame();
                } else if (pt.y >= 360) { // Main Menu
                    currentMode = MODE_ARCADE_HUB;
                    drawArcadeHub();
                }
                Touch.reset();
                prevTouchPressed = true;
                return;
            }
        } else if (Snake.getState() == SNAKE_STATE_MENU) {
            if (justTapped) {
                Snake.handleMenuTouch(pt.x, pt.y);
                Touch.reset();
                prevTouchPressed = true;
                return;
            }
        } else if (Snake.getState() == SNAKE_STATE_PLAYING) {
            // Gyro Steering (Rotate/tilt watch in real-time to steer the snake!)
            IMU.update();
            int8_t quadrant = IMU.getCurrentQuadrant();
            Snake.setDirectionFromQuadrant(quadrant);

            // Optional Touch Gestures (Swipe to steer)
            Touch.processGestures(deltaX, deltaY);
            if (deltaX != 0 || deltaY != 0) {
                Snake.handleSwipe(deltaX, deltaY);
            }
        }
    }
}

void loop() {
    uint32_t now = millis();
    uint32_t delta = now - lastFrameTime;
    lastFrameTime = now;

    handleInputs();

    if (currentMode == MODE_TETRIS) {
        Game.update(delta);
    } else if (currentMode == MODE_SNAKE) {
        Snake.update(delta);
    }

    delay(5);
}
