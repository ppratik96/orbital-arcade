#pragma once

#include <Arduino.h>
#include "config.h"

enum GravityDirection {
    GRAVITY_DOWN = 0,  // Bottom Well
    GRAVITY_LEFT,      // Left Well
    GRAVITY_UP,        // Top Well
    GRAVITY_RIGHT      // Right Well
};

struct Piece {
    int8_t matrix[4][4];
    uint8_t size;
    int8_t x;
    int8_t y;
    uint8_t typeId;
    uint16_t color;
};

enum GameState {
    STATE_START_MENU = 0,
    STATE_PLAYING,
    STATE_GAME_OVER
};

class GameEngine {
public:
    GameEngine();
    void init();
    void resetGame();
    void showStartScreen();
    void setDifficulty(Difficulty diff);
    void handleMenuTouch(int16_t x, int16_t y);
    void update(uint32_t deltaMs);

    // Player Actions
    void movePiece(int8_t delta);
    void dropPiece();
    void hardDrop();
    void setGravityDirection(GravityDirection dir);

    // Render methods
    void renderAll();
    void renderDynamic();
    void redrawBoard();
    void drawPlayfieldBackground();
    void drawHUD();
    void drawGameOverScreen();
    void drawStartScreen();
    void drawDifficultyButtons();

    // State Getters
    GameState getState() const { return _state; }
    Difficulty getDifficulty() const { return _difficulty; }
    uint32_t getScore() const { return _score; }
    uint32_t getHighScore() const { return _highScore; }
    GravityDirection getGravityDirection() const { return _gravityDir; }
    int16_t getRotationAngle() const { return (int16_t)_gravityDir * 90; }

    bool returnToHubRequested;

private:
    int8_t _board[GRID_ROWS][GRID_COLS];
    Piece _currentPiece;
    Piece _prevPiece;
    int8_t _prevGhostX;
    int8_t _prevGhostY;
    bool _hasPrevPiece;
    GameState _state;
    Difficulty _difficulty;
    GravityDirection _gravityDir;

    uint32_t _score;
    uint32_t _highScore;
    uint32_t _dropCounter;
    uint32_t _lockTimer;
    uint8_t _lockMoves;
    bool _isGrounded;
    uint32_t _lastRotateTime;

    void initEmptyBoard();
    void spawnPiece();
    void createPiece(Piece &p, uint8_t typeId);
    bool collides(int8_t offsetX, int8_t offsetY, const int8_t matrix[4][4], uint8_t size, int8_t posX, int8_t posY);
    void mergePiece();
    void clearLines();
    void getGhostPos(int8_t &gx, int8_t &gy);
    void getGravityVector(int8_t &gx, int8_t &gy);
    void eraseTile(int8_t r, int8_t c);
    uint32_t getCurrentSpeed();
    bool isCenterBoxBreached();
    void drawCountdownDigit(const char *digit, uint16_t color);
    void clearCountdownBox();
};

extern GameEngine Game;
