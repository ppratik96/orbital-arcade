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

    // Outer Cyber Corner Accents
    Display.drawRect(8, 8, SCREEN_WIDTH - 16, SCREEN_HEIGHT - 16, 0x1A0C);
    Display.drawRect(10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20, COLOR_TEXT_CYAN);
    
    // Header
    Display.drawCenteredString(20, "ORBITAL ARCADE", COLOR_TEXT_CYAN, COLOR_BG, 3);
    Display.drawFastHLine(30, 48, SCREEN_WIDTH - 60, 0x07BF);
    Display.drawCenteredString(54, "-- SELECT CARTRIDGE --", COLOR_TEXT_GRAY, COLOR_BG, 1);

    // =========================================================================
    // Card 1: ROTATETRIS (Y = 75, H = 150)
    // =========================================================================
    int16_t c1Y = 75;
    Display.fillRect(20, c1Y, SCREEN_WIDTH - 40, 150, 0x0845);
    Display.drawRect(20, c1Y, SCREEN_WIDTH - 40, 150, COLOR_TEXT_CYAN);
    Display.drawRect(21, c1Y + 1, SCREEN_WIDTH - 42, 148, 0x07BF);

    // --- Mini Tetris Icon (Left: X = 32..82, Y = 86..140) ---
    // T-piece (Cyan, 11px blocks)
    Display.drawBeveledBlock(48, c1Y + 12, 11, COLOR_CYAN, false);
    Display.drawBeveledBlock(37, c1Y + 23, 11, COLOR_CYAN, false);
    Display.drawBeveledBlock(48, c1Y + 23, 11, COLOR_CYAN, false);
    Display.drawBeveledBlock(59, c1Y + 23, 11, COLOR_CYAN, false);

    // L-piece (Amber, 11px blocks)
    Display.drawBeveledBlock(68, c1Y + 36, 11, COLOR_AMBER, false);
    Display.drawBeveledBlock(68, c1Y + 47, 11, COLOR_AMBER, false);
    Display.drawBeveledBlock(79, c1Y + 47, 11, COLOR_AMBER, false);

    // --- Card 1 Text ---
    Display.drawString(100, c1Y + 12, "ROTATETRIS", COLOR_TEXT_CYAN, 0x0845, 2);
    Display.drawString(100, c1Y + 34, "4-Way Gravity Stacker", COLOR_TEXT_GRAY, 0x0845, 1);
    
    char tetrisHi[32];
    snprintf(tetrisHi, sizeof(tetrisHi), "BEST: %lu PTS", SDConfig.getConfig().highScore);
    Display.drawString(100, c1Y + 50, tetrisHi, COLOR_TEXT_AMBER, 0x0845, 1);

    // --- Real Launch Button 1 ---
    int16_t btn1Y = c1Y + 95;
    Display.fillRect(32, btn1Y, SCREEN_WIDTH - 64, 42, COLOR_TEXT_CYAN);
    Display.drawRect(30, btn1Y - 2, SCREEN_WIDTH - 60, 46, 0xFFFF);
    Display.drawCenteredString(btn1Y + 12, "PLAY ROTATETRIS", 0x0000, COLOR_TEXT_CYAN, 2);

    // =========================================================================
    // Card 2: ROSNAKE (Y = 240, H = 150)
    // =========================================================================
    int16_t c2Y = 240;
    Display.fillRect(20, c2Y, SCREEN_WIDTH - 40, 150, 0x0182);
    Display.drawRect(20, c2Y, SCREEN_WIDTH - 40, 150, COLOR_TEXT_GREEN);
    Display.drawRect(21, c2Y + 1, SCREEN_WIDTH - 42, 148, 0x07E0);

    // --- Mini Snake Icon (Left: X = 32..85, Y = 252..305) ---
    // Food Orb (Gold, 11px)
    Display.drawBeveledBlock(76, c2Y + 14, 11, COLOR_AMBER, false);

    // Snake Head (Green, 11px) with eyes
    Display.drawBeveledBlock(60, c2Y + 14, 11, COLOR_SNAKE_HEAD, false);
    Display.fillRect(68, c2Y + 16, 2, 2, 0x0000);
    Display.fillRect(68, c2Y + 21, 2, 2, 0x0000);

    // Snake Body Segments (11px)
    Display.drawBeveledBlock(49, c2Y + 14, 11, COLOR_SNAKE_BODY, false);
    Display.drawBeveledBlock(38, c2Y + 14, 11, COLOR_SNAKE_BODY, false);
    Display.drawBeveledBlock(38, c2Y + 25, 11, COLOR_SNAKE_BODY, false);
    Display.drawBeveledBlock(38, c2Y + 36, 11, COLOR_SNAKE_BODY, false);
    Display.drawBeveledBlock(49, c2Y + 36, 11, COLOR_SNAKE_BODY, false);

    // --- Card 2 Text ---
    Display.drawString(100, c2Y + 12, "ROSNAKE", COLOR_TEXT_GREEN, 0x0182, 2);
    Display.drawString(100, c2Y + 34, "Gyro Tilt-Steered", COLOR_TEXT_GRAY, 0x0182, 1);
    
    char snakeHi[32];
    snprintf(snakeHi, sizeof(snakeHi), "BEST: %lu PTS", SDConfig.getConfig().snakeHighScore);
    Display.drawString(100, c2Y + 50, snakeHi, COLOR_TEXT_AMBER, 0x0182, 1);

    // --- Real Launch Button 2 ---
    int16_t btn2Y = c2Y + 95;
    Display.fillRect(32, btn2Y, SCREEN_WIDTH - 64, 42, COLOR_TEXT_GREEN);
    Display.drawRect(30, btn2Y - 2, SCREEN_WIDTH - 60, 46, 0xFFFF);
    Display.drawCenteredString(btn2Y + 12, "PLAY ROSNAKE", 0x0000, COLOR_TEXT_GREEN, 2);

    // =========================================================================
    // Footer
    // =========================================================================
    Display.drawCenteredString(410, "TAP BUTTON TO LAUNCH", COLOR_TEXT_GRAY, COLOR_BG, 1);
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
