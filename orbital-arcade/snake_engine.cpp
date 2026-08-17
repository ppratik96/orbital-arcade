#include "snake_engine.h"
#include "display_amoled.h"
#include "sd_config.h"

SnakeEngine Snake;

#ifndef COLOR_SNAKE_HEAD
#define COLOR_SNAKE_HEAD   0x07FF  // Vibrant Electric Cyan
#endif
#ifndef COLOR_SNAKE_BODY
#define COLOR_SNAKE_BODY   0x07E0  // Neon Green
#endif
#ifndef COLOR_SNAKE_TAIL
#define COLOR_SNAKE_TAIL   0x05E0  // Emerald Green
#endif
#ifndef COLOR_SNAKE_FOOD
#define COLOR_SNAKE_FOOD   0xFFE0  // Glowing Gold Energy Orb
#endif
#ifndef COLOR_SNAKE_BONUS
#define COLOR_SNAKE_BONUS  0xF81F  // Magenta Bonus Core
#endif
#define COLOR_SNAKE_GRID   0x1904  // Subtle arena grid
#define COLOR_SNAKE_BORDER 0x07E0  // Neon Green Cyber Border

SnakeEngine::SnakeEngine() :
    _length(4),
    _dir(SNAKE_DIR_RIGHT),
    _nextDir(SNAKE_DIR_RIGHT),
    _hasBonus(false),
    _bonusTimer(0),
    _state(SNAKE_STATE_MENU),
    _difficulty(DIFF_MEDIUM),
    _score(0),
    _highScore(0),
    _stepTimer(0),
    _foodCount(0),
    returnToHubRequested(false) {}

void SnakeEngine::init() {
    _highScore = SDConfig.getConfig().snakeHighScore;
}

void SnakeEngine::showStartScreen() {
    _state = SNAKE_STATE_MENU;
    returnToHubRequested = false;
    drawStartScreen();
}

void SnakeEngine::spawnFood() {
    bool valid = false;
    while (!valid) {
        _food.x = random(0, SNAKE_GRID_COLS);
        _food.y = random(0, SNAKE_GRID_ROWS);
        valid = true;
        for (uint16_t i = 0; i < _length; i++) {
            if (_body[i].x == _food.x && _body[i].y == _food.y) {
                valid = false;
                break;
            }
        }
    }
}

void SnakeEngine::spawnBonusFood() {
    bool valid = false;
    while (!valid) {
        _bonusFood.x = random(0, SNAKE_GRID_COLS);
        _bonusFood.y = random(0, SNAKE_GRID_ROWS);
        valid = true;
        if (_bonusFood.x == _food.x && _bonusFood.y == _food.y) valid = false;
        for (uint16_t i = 0; i < _length; i++) {
            if (_body[i].x == _bonusFood.x && _body[i].y == _bonusFood.y) {
                valid = false;
                break;
            }
        }
    }
    _hasBonus = true;
    _bonusTimer = 6000; // 6 seconds before expiring
}

uint32_t SnakeEngine::getStepSpeed() {
    if (_difficulty == DIFF_EASY) {
        return max(80, (int)(200 - (_foodCount * 3)));
    } else if (_difficulty == DIFF_HARD) {
        return max(40, (int)(85 - (_foodCount * 2)));
    } else { // MEDIUM
        return max(55, (int)(140 - (_foodCount * 3)));
    }
}

void SnakeEngine::drawCell(int8_t x, int8_t y, uint16_t color, bool isHead) {
    if (x < 0 || x >= SNAKE_GRID_COLS || y < 0 || y >= SNAKE_GRID_ROWS) return;
    int16_t px = SNAKE_PLAYFIELD_X + (x * SNAKE_BLOCK_SIZE);
    int16_t py = SNAKE_PLAYFIELD_Y + (y * SNAKE_BLOCK_SIZE);

    if (isHead) {
        // Head with eye accents
        Display.drawBeveledBlock(px, py, SNAKE_BLOCK_SIZE, color, false);
        // Eyes
        int16_t e1x = px + 5, e1y = py + 5;
        int16_t e2x = px + SNAKE_BLOCK_SIZE - 9, e2y = py + 5;
        if (_dir == SNAKE_DIR_DOWN) {
            e1y = py + SNAKE_BLOCK_SIZE - 9;
            e2y = py + SNAKE_BLOCK_SIZE - 9;
        } else if (_dir == SNAKE_DIR_LEFT) {
            e1x = px + 5; e1y = py + 5;
            e2x = px + 5; e2y = py + SNAKE_BLOCK_SIZE - 9;
        } else if (_dir == SNAKE_DIR_RIGHT) {
            e1x = px + SNAKE_BLOCK_SIZE - 9; e1y = py + 5;
            e2x = px + SNAKE_BLOCK_SIZE - 9; e2y = py + SNAKE_BLOCK_SIZE - 9;
        }
        Display.fillRect(e1x, e1y, 4, 4, 0x0000);
        Display.fillRect(e2x, e2y, 4, 4, 0x0000);
    } else {
        Display.drawBeveledBlock(px, py, SNAKE_BLOCK_SIZE, color, false);
    }
}

void SnakeEngine::eraseCell(int8_t x, int8_t y) {
    if (x < 0 || x >= SNAKE_GRID_COLS || y < 0 || y >= SNAKE_GRID_ROWS) return;
    int16_t px = SNAKE_PLAYFIELD_X + (x * SNAKE_BLOCK_SIZE);
    int16_t py = SNAKE_PLAYFIELD_Y + (y * SNAKE_BLOCK_SIZE);
    Display.fillRect(px, py, SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, COLOR_BG);
    Display.drawRect(px, py, SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, COLOR_SNAKE_GRID);
}

void SnakeEngine::drawCountdownDigit(const char *digit, uint16_t color) {
    int16_t cx = SNAKE_PLAYFIELD_X + (SNAKE_GRID_COLS * SNAKE_BLOCK_SIZE) / 2 - 40;
    int16_t cy = SNAKE_PLAYFIELD_Y + (SNAKE_GRID_ROWS * SNAKE_BLOCK_SIZE) / 2 - 30;
    Display.fillRect(cx, cy, 80, 60, COLOR_BG);
    Display.drawRect(cx, cy, 80, 60, color);
    Display.drawCenteredString(cy + 12, digit, color, COLOR_BG, 5);
}

void SnakeEngine::resetGame() {
    _length = 4;
    _dir = SNAKE_DIR_RIGHT;
    _nextDir = SNAKE_DIR_RIGHT;
    _score = 0;
    _stepTimer = 0;
    _foodCount = 0;
    _hasBonus = false;
    _bonusTimer = 0;
    returnToHubRequested = false;

    // Start in center (6, 6)
    _body[0] = {6, 6};
    _body[1] = {5, 6};
    _body[2] = {4, 6};
    _body[3] = {3, 6};

    spawnFood();

    // 1. Draw Playfield & HUD
    Display.fillScreen(COLOR_BG);
    drawHUD();
    drawPlayfield();

    // 2. Countdown 3, 2, 1
    drawCountdownDigit("3", COLOR_TEXT_CYAN);
    delay(500);

    drawCountdownDigit("2", COLOR_TEXT_AMBER);
    delay(500);

    drawCountdownDigit("1", COLOR_TEXT_GREEN);
    delay(500);

    // Redraw clean arena playfield
    drawPlayfield();

    // Draw initial snake
    for (uint16_t i = 1; i < _length; i++) {
        drawCell(_body[i].x, _body[i].y, COLOR_SNAKE_BODY, false);
    }
    drawCell(_body[0].x, _body[0].y, COLOR_SNAKE_HEAD, true);

    // Draw initial food
    drawCell(_food.x, _food.y, COLOR_SNAKE_FOOD, false);

    _state = SNAKE_STATE_PLAYING;
}

void SnakeEngine::drawPlayfield() {
    int16_t sx = SNAKE_PLAYFIELD_X;
    int16_t sy = SNAKE_PLAYFIELD_Y;
    int16_t totalW = SNAKE_GRID_COLS * SNAKE_BLOCK_SIZE;
    int16_t totalH = SNAKE_GRID_ROWS * SNAKE_BLOCK_SIZE;

    Display.fillRect(sx, sy, totalW, totalH, COLOR_BG);

    // Subtle grid lines
    for (int r = 0; r < SNAKE_GRID_ROWS; r++) {
        for (int c = 0; c < SNAKE_GRID_COLS; c++) {
            Display.drawRect(sx + (c * SNAKE_BLOCK_SIZE), sy + (r * SNAKE_BLOCK_SIZE), SNAKE_BLOCK_SIZE, SNAKE_BLOCK_SIZE, COLOR_SNAKE_GRID);
        }
    }

    // Outer Glowing Border
    Display.drawRect(sx - 2, sy - 2, totalW + 4, totalH + 4, COLOR_SNAKE_BORDER);
    Display.drawRect(sx - 1, sy - 1, totalW + 2, totalH + 2, COLOR_SNAKE_BORDER);
}

void SnakeEngine::drawHUD() {
    char buf[32];
    snprintf(buf, sizeof(buf), "SCORE:%lu", _score);
    Display.drawString(10, 18, buf, COLOR_TEXT_GREEN, COLOR_BG, 2);

    if (_difficulty == DIFF_EASY) {
        Display.drawString(160, 18, "EASY", COLOR_TEXT_GREEN, COLOR_BG, 2);
    } else if (_difficulty == DIFF_HARD) {
        Display.drawString(160, 18, "HARD", COLOR_TEXT_RED, COLOR_BG, 2);
    } else {
        Display.drawString(160, 18, "MED", COLOR_TEXT_AMBER, COLOR_BG, 2);
    }

    Display.drawString(SCREEN_WIDTH - 150, 18, "ROSNAKE", COLOR_TEXT_GREEN, COLOR_BG, 2);
}

void SnakeEngine::setDirection(SnakeDirection dir) {
    if (_state != SNAKE_STATE_PLAYING) return;

    // Prevent 180-degree self-collision turns
    if (_dir == SNAKE_DIR_UP && dir == SNAKE_DIR_DOWN) return;
    if (_dir == SNAKE_DIR_DOWN && dir == SNAKE_DIR_UP) return;
    if (_dir == SNAKE_DIR_LEFT && dir == SNAKE_DIR_RIGHT) return;
    if (_dir == SNAKE_DIR_RIGHT && dir == SNAKE_DIR_LEFT) return;

    _nextDir = dir;
}

void SnakeEngine::setDirectionFromQuadrant(int8_t quadrant) {
    if (_state != SNAKE_STATE_PLAYING) return;

    switch (quadrant) {
        case 0: setDirection(SNAKE_DIR_DOWN); break;  // GRAVITY_DOWN
        case 1: setDirection(SNAKE_DIR_LEFT); break;  // GRAVITY_LEFT
        case 2: setDirection(SNAKE_DIR_UP); break;    // GRAVITY_UP
        case 3: setDirection(SNAKE_DIR_RIGHT); break; // GRAVITY_RIGHT
    }
}

void SnakeEngine::handleSwipe(int16_t deltaX, int16_t deltaY) {
    if (abs(deltaX) > abs(deltaY)) {
        if (deltaX > 0) setDirection(SNAKE_DIR_RIGHT);
        else if (deltaX < 0) setDirection(SNAKE_DIR_LEFT);
    } else {
        if (deltaY > 0) setDirection(SNAKE_DIR_DOWN);
        else if (deltaY < 0) setDirection(SNAKE_DIR_UP);
    }
}

void SnakeEngine::step() {
    _dir = _nextDir;

    int8_t nextHeadX = _body[0].x;
    int8_t nextHeadY = _body[0].y;

    switch (_dir) {
        case SNAKE_DIR_UP:    nextHeadY--; break;
        case SNAKE_DIR_DOWN:  nextHeadY++; break;
        case SNAKE_DIR_LEFT:  nextHeadX--; break;
        case SNAKE_DIR_RIGHT: nextHeadX++; break;
    }

    // Wall Collision Check
    if (nextHeadX < 0 || nextHeadX >= SNAKE_GRID_COLS || nextHeadY < 0 || nextHeadY >= SNAKE_GRID_ROWS) {
        _state = SNAKE_STATE_GAME_OVER;
        if (_score > _highScore) {
            _highScore = _score;
            SDConfig.saveSnakeHighScore(_highScore);
        }
        drawGameOverScreen();
        return;
    }

    // Self Collision Check (excluding tail if it's going to move)
    bool eatsFood = (nextHeadX == _food.x && nextHeadY == _food.y);
    uint16_t checkLength = eatsFood ? _length : _length - 1;
    for (uint16_t i = 0; i < checkLength; i++) {
        if (_body[i].x == nextHeadX && _body[i].y == nextHeadY) {
            _state = SNAKE_STATE_GAME_OVER;
            if (_score > _highScore) {
                _highScore = _score;
                SDConfig.saveSnakeHighScore(_highScore);
            }
            drawGameOverScreen();
            return;
        }
    }

    // Bonus Food Collision
    bool eatsBonus = (_hasBonus && nextHeadX == _bonusFood.x && nextHeadY == _bonusFood.y);
    if (eatsBonus) {
        _score += 500;
        _hasBonus = false;
        drawHUD();
    }

    if (eatsFood) {
        if (_length < SNAKE_MAX_LENGTH) {
            _length++;
        }
        uint32_t baseScore = (_difficulty == DIFF_EASY ? 100 : (_difficulty == DIFF_MEDIUM ? 150 : 250));
        _score += baseScore;
        _foodCount++;
        drawHUD();

        spawnFood();
        drawCell(_food.x, _food.y, COLOR_SNAKE_FOOD, false);

        // Spawn bonus every 5 foods
        if (_foodCount % 5 == 0 && !_hasBonus) {
            spawnBonusFood();
            drawCell(_bonusFood.x, _bonusFood.y, COLOR_SNAKE_BONUS, false);
        }
    } else {
        // Erase old tail
        eraseCell(_body[_length - 1].x, _body[_length - 1].y);
    }

    // Shift body
    for (int i = _length - 1; i > 0; i--) {
        _body[i] = _body[i - 1];
    }
    _body[0] = {nextHeadX, nextHeadY};

    // Draw neck segment (now body)
    if (_length > 1) {
        drawCell(_body[1].x, _body[1].y, COLOR_SNAKE_BODY, false);
    }
    // Draw new head
    drawCell(_body[0].x, _body[0].y, COLOR_SNAKE_HEAD, true);
}

void SnakeEngine::update(uint32_t deltaMs) {
    if (_state != SNAKE_STATE_PLAYING) return;

    if (_hasBonus) {
        if (_bonusTimer > deltaMs) {
            _bonusTimer -= deltaMs;
        } else {
            _hasBonus = false;
            eraseCell(_bonusFood.x, _bonusFood.y);
        }
    }

    _stepTimer += deltaMs;
    uint32_t speed = getStepSpeed();
    if (_stepTimer >= speed) {
        step();
        _stepTimer = 0;
    }
}

void SnakeEngine::drawDifficultyButtons() {
    bool isEasy = (_difficulty == DIFF_EASY);
    uint16_t easyBg = isEasy ? COLOR_TEXT_GREEN : COLOR_BG;
    uint16_t easyFg = isEasy ? COLOR_BG : COLOR_TEXT_GREEN;
    Display.fillRect(30, 95, SCREEN_WIDTH - 60, 48, easyBg);
    Display.drawRect(30, 95, SCREEN_WIDTH - 60, 48, COLOR_TEXT_GREEN);
    Display.drawRect(31, 96, SCREEN_WIDTH - 62, 46, COLOR_TEXT_GREEN);
    Display.drawCenteredString(108, "EASY  (200ms)", easyFg, easyBg, 2);

    bool isMed = (_difficulty == DIFF_MEDIUM);
    uint16_t medBg = isMed ? COLOR_TEXT_AMBER : COLOR_BG;
    uint16_t medFg = isMed ? COLOR_BG : COLOR_TEXT_AMBER;
    Display.fillRect(30, 155, SCREEN_WIDTH - 60, 48, medBg);
    Display.drawRect(30, 155, SCREEN_WIDTH - 60, 48, COLOR_TEXT_AMBER);
    Display.drawRect(31, 156, SCREEN_WIDTH - 62, 46, COLOR_TEXT_AMBER);
    Display.drawCenteredString(168, "MEDIUM  (140ms)", medFg, medBg, 2);

    bool isHard = (_difficulty == DIFF_HARD);
    uint16_t hardBg = isHard ? COLOR_TEXT_RED : COLOR_BG;
    uint16_t hardFg = isHard ? COLOR_BG : COLOR_TEXT_RED;
    Display.fillRect(30, 215, SCREEN_WIDTH - 60, 48, hardBg);
    Display.drawRect(30, 215, SCREEN_WIDTH - 60, 48, COLOR_TEXT_RED);
    Display.drawRect(31, 216, SCREEN_WIDTH - 62, 46, COLOR_TEXT_RED);
    Display.drawCenteredString(228, "HARD  (85ms)", hardFg, hardBg, 2);
}

void SnakeEngine::setDifficulty(Difficulty diff) {
    if (_difficulty == diff) return;
    _difficulty = diff;
    drawDifficultyButtons();
}

void SnakeEngine::handleMenuTouch(int16_t x, int16_t y) {
    if (y >= 85 && y <= 148) {
        setDifficulty(DIFF_EASY);
    } else if (y >= 149 && y <= 208) {
        setDifficulty(DIFF_MEDIUM);
    } else if (y >= 209 && y <= 270) {
        setDifficulty(DIFF_HARD);
    } else if (y >= 330) {
        resetGame();
    }
}

void SnakeEngine::drawStartScreen() {
    Display.fillScreen(COLOR_BG);

    // Outer Cyber Green Outline
    Display.drawRect(10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20, COLOR_TEXT_GREEN);
    Display.drawRect(12, 12, SCREEN_WIDTH - 24, SCREEN_HEIGHT - 24, COLOR_TEXT_GREEN);

    // Title
    Display.drawCenteredString(30, "ROSNAKE", COLOR_TEXT_GREEN, COLOR_BG, 3);
    Display.drawCenteredString(65, "TILT WATCH TO STEER", COLOR_TEXT_GRAY, COLOR_BG, 1);

    drawDifficultyButtons();

    // High Score Record
    char hiBuf[32];
    snprintf(hiBuf, sizeof(hiBuf), "RECORD: %lu", _highScore);
    Display.drawCenteredString(290, hiBuf, COLOR_TEXT_WHITE, COLOR_BG, 2);

    // Launch Button
    Display.fillRect(30, 345, SCREEN_WIDTH - 60, 58, COLOR_TEXT_GREEN);
    Display.drawCenteredString(363, "START MISSION", COLOR_BG, COLOR_TEXT_GREEN, 3);
}

void SnakeEngine::drawGameOverScreen() {
    Display.fillScreen(COLOR_BG);

    Display.drawRect(12, 12, SCREEN_WIDTH - 24, SCREEN_HEIGHT - 24, COLOR_TEXT_RED);
    Display.drawRect(14, 14, SCREEN_WIDTH - 28, SCREEN_HEIGHT - 28, COLOR_TEXT_RED);

    Display.drawCenteredString(45, "MISSION FAILED", COLOR_TEXT_RED, COLOR_BG, 3);
    Display.drawCenteredString(80, "COLLISION DETECTED", COLOR_TEXT_AMBER, COLOR_BG, 1);

    Display.fillRect(30, 105, SCREEN_WIDTH - 60, 2, COLOR_TEXT_RED);

    Display.drawCenteredString(130, "FINAL SCORE", COLOR_TEXT_GRAY, COLOR_BG, 1);
    char buf[32];
    snprintf(buf, sizeof(buf), "%lu", _score);
    Display.drawCenteredString(160, buf, COLOR_TEXT_WHITE, COLOR_BG, 4);

    char hiBuf[32];
    snprintf(hiBuf, sizeof(hiBuf), "BEST RECORD: %lu", _highScore);
    Display.drawCenteredString(240, hiBuf, COLOR_TEXT_GREEN, COLOR_BG, 2);

    Display.fillRect(30, 280, SCREEN_WIDTH - 60, 2, COLOR_TEXT_RED);

    // Button 1: RETRY (Y = 300..355)
    Display.fillRect(35, 300, SCREEN_WIDTH - 70, 50, COLOR_TEXT_GREEN);
    Display.drawCenteredString(316, "PLAY AGAIN", COLOR_BG, COLOR_TEXT_GREEN, 2);

    // Button 2: MAIN MENU (Y = 365..420)
    Display.fillRect(35, 365, SCREEN_WIDTH - 70, 50, COLOR_TEXT_CYAN);
    Display.drawCenteredString(381, "MAIN MENU", COLOR_BG, COLOR_TEXT_CYAN, 2);
}
