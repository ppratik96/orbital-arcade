#pragma once

#include <Arduino.h>
#include "config.h"

#define SNAKE_GRID_COLS     13
#define SNAKE_GRID_ROWS     13
#define SNAKE_BLOCK_SIZE    26   // 13 * 26 = 338 px (Exact match to Rotatetris!)
#define SNAKE_PLAYFIELD_X   15   // (368 - 338) / 2 = 15 px
#define SNAKE_PLAYFIELD_Y   55   // Top offset below HUD

#define SNAKE_MAX_LENGTH    (SNAKE_GRID_COLS * SNAKE_GRID_ROWS)

enum SnakeDirection {
    SNAKE_DIR_UP = 0,
    SNAKE_DIR_RIGHT,
    SNAKE_DIR_DOWN,
    SNAKE_DIR_LEFT
};

enum SnakeGameState {
    SNAKE_STATE_MENU = 0,
    SNAKE_STATE_PLAYING,
    SNAKE_STATE_GAME_OVER
};

struct SnakePoint {
    int8_t x;
    int8_t y;
};

class SnakeEngine {
public:
    SnakeEngine();
    void init();
    void resetGame();
    void showStartScreen();
    void setDifficulty(Difficulty diff);
    void handleMenuTouch(int16_t x, int16_t y);
    void update(uint32_t deltaMs);

    // Controls
    void setDirection(SnakeDirection dir);
    void setDirectionFromQuadrant(int8_t quadrant);
    void handleSwipe(int16_t deltaX, int16_t deltaY);

    // Rendering
    void drawHUD();
    void drawPlayfield();
    void drawStartScreen();
    void drawDifficultyButtons();
    void drawGameOverScreen();
    void drawCountdownDigit(const char *digit, uint16_t color);

    // State Getters
    SnakeGameState getState() const { return _state; }
    uint32_t getScore() const { return _score; }
    uint32_t getHighScore() const { return _highScore; }
    Difficulty getDifficulty() const { return _difficulty; }

    bool returnToHubRequested;

private:
    SnakePoint _body[SNAKE_MAX_LENGTH];
    uint16_t _length;
    SnakeDirection _dir;
    SnakeDirection _nextDir;
    
    SnakePoint _food;
    SnakePoint _bonusFood;
    bool _hasBonus;
    uint32_t _bonusTimer;

    SnakeGameState _state;
    Difficulty _difficulty;
    uint32_t _score;
    uint32_t _highScore;
    uint32_t _stepTimer;
    uint32_t _foodCount;

    void spawnFood();
    void spawnBonusFood();
    void step();
    uint32_t getStepSpeed();
    void drawCell(int8_t x, int8_t y, uint16_t color, bool isHead = false);
    void eraseCell(int8_t x, int8_t y);
};

extern SnakeEngine Snake;
