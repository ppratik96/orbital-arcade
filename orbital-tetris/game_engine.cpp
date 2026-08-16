#include "game_engine.h"
#include "display_amoled.h"
#include "sd_config.h"
#include "pmu_axp2101.h"

GameEngine Game;

// Tetromino colors matching reference screenshot (1:1)
static const uint16_t PIECE_COLORS[8] = {
    COLOR_BG,
    0x073F, // 1: I - Cyan (#00e5ff)
    0x1BAF, // 2: J - Blue (#1a75ff)
    0xFC80, // 3: L - Orange (#ff9900)
    0xFFE0, // 4: O - Yellow (#ffff00)
    0x0720, // 5: S - Green (#00e600)
    0x9813, // 6: T - Purple (#990099)
    0xF8A3  // 7: Z - Red (#ff1a1a)
};

GameEngine::GameEngine() : 
    _hasPrevPiece(false),
    _prevGhostX(-1),
    _prevGhostY(-1),
    _state(STATE_START_MENU), 
    _difficulty(DIFF_MEDIUM),
    _gravityDir(GRAVITY_DOWN),
    _score(0), 
    _highScore(0), 
    _dropCounter(0), 
    _lockTimer(0),
    _lockMoves(0),
    _isGrounded(false),
    _lastRotateTime(0) {}

void GameEngine::init() {
    _highScore = SDConfig.getConfig().highScore;
    initEmptyBoard();
}

void GameEngine::initEmptyBoard() {
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            // 3x3 Corners (9 cells per corner)
            if (r < 3 && c < 3) _board[r][c] = -1;
            else if (r < 3 && c > 9) _board[r][c] = -1;
            else if (r > 9 && c < 3) _board[r][c] = -1;
            else if (r > 9 && c > 9) _board[r][c] = -1;
            else _board[r][c] = 0;
        }
    }
    _hasPrevPiece = false;
    _isGrounded = false;
    _lockTimer = 0;
    _lockMoves = 0;
}

void GameEngine::createPiece(Piece &p, uint8_t typeId) {
    p.typeId = typeId;
    p.color = PIECE_COLORS[typeId];
    memset(p.matrix, 0, sizeof(p.matrix));

    switch (typeId) {
        case 1: // I
            p.size = 4;
            p.matrix[1][0] = 1; p.matrix[1][1] = 1; p.matrix[1][2] = 1; p.matrix[1][3] = 1;
            break;
        case 2: // J
            p.size = 3;
            p.matrix[0][0] = 2; p.matrix[1][0] = 2; p.matrix[1][1] = 2; p.matrix[1][2] = 2;
            break;
        case 3: // L
            p.size = 3;
            p.matrix[0][2] = 3; p.matrix[1][0] = 3; p.matrix[1][1] = 3; p.matrix[1][2] = 3;
            break;
        case 4: // O
            p.size = 2;
            p.matrix[0][0] = 4; p.matrix[0][1] = 4; p.matrix[1][0] = 4; p.matrix[1][1] = 4;
            break;
        case 5: // S
            p.size = 3;
            p.matrix[0][1] = 5; p.matrix[0][2] = 5; p.matrix[1][0] = 5; p.matrix[1][1] = 5;
            break;
        case 6: // T
            p.size = 3;
            p.matrix[0][1] = 6; p.matrix[1][0] = 6; p.matrix[1][1] = 6; p.matrix[1][2] = 6;
            break;
        case 7: // Z
            p.size = 3;
            p.matrix[0][0] = 7; p.matrix[0][1] = 7; p.matrix[1][1] = 7; p.matrix[1][2] = 7;
            break;
        default:
            p.size = 3;
            break;
    }
}

bool GameEngine::isCenterBoxBreached() {
    for (int r = CORE_R_MIN; r <= CORE_R_MAX; r++) {
        for (int c = CORE_C_MIN; c <= CORE_C_MAX; c++) {
            if (_board[r][c] > 0) {
                return true;
            }
        }
    }
    return false;
}

void GameEngine::spawnPiece() {
    _hasPrevPiece = false;
    _isGrounded = false;
    _lockTimer = 0;
    _lockMoves = 0;

    uint8_t typeId = random(1, 8);
    createPiece(_currentPiece, typeId);

    // All pieces spawn directly inside the 3x3 Central Core Box (rows 5..7, cols 5..7)
    if (_currentPiece.size == 4) { // I piece
        _currentPiece.x = 5;
        _currentPiece.y = 5;
    } else if (_currentPiece.size == 2) { // O piece
        _currentPiece.x = 6;
        _currentPiece.y = 6;
    } else { // 3x3 pieces: J, L, S, T, Z
        _currentPiece.x = 5;
        _currentPiece.y = 5;
    }

    // 3x3 Central Box is the Ceiling for Overflow:
    // If locked blocks have entered the 3x3 box, or if spawn collides with existing blocks:
    if (isCenterBoxBreached() || collides(0, 0, _currentPiece.matrix, _currentPiece.size, _currentPiece.x, _currentPiece.y)) {
        _state = STATE_GAME_OVER;
        if (_score > _highScore) {
            _highScore = _score;
            SDConfig.saveHighScore(_highScore);
        }
        drawGameOverScreen();
        return;
    }
}

bool GameEngine::collides(int8_t offsetX, int8_t offsetY, const int8_t matrix[4][4], uint8_t size, int8_t posX, int8_t posY) {
    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            if (matrix[r][c] != 0) {
                int8_t newY = posY + r + offsetY;
                int8_t newX = posX + c + offsetX;

                if (newY < 0 || newY >= GRID_ROWS || newX < 0 || newX >= GRID_COLS) {
                    return true;
                }

                if (_board[newY][newX] != 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

void GameEngine::getGravityVector(int8_t &gx, int8_t &gy) {
    gx = 0; gy = 0;
    switch (_gravityDir) {
        case GRAVITY_DOWN:  gx = 0;  gy = 1;  break;
        case GRAVITY_LEFT:  gx = -1; gy = 0;  break;
        case GRAVITY_UP:    gx = 0;  gy = -1; break;
        case GRAVITY_RIGHT: gx = 1;  gy = 0;  break;
    }
}

void GameEngine::movePiece(int8_t lateral) {
    if (_state != STATE_PLAYING) return;

    int8_t dx = 0, dy = 0;
    if (_gravityDir == GRAVITY_DOWN) {
        dx = lateral;
    } else if (_gravityDir == GRAVITY_UP) {
        dx = -lateral;
    } else if (_gravityDir == GRAVITY_LEFT) {
        dy = lateral;
    } else if (_gravityDir == GRAVITY_RIGHT) {
        dy = -lateral;
    }

    if (!collides(dx, dy, _currentPiece.matrix, _currentPiece.size, _currentPiece.x, _currentPiece.y)) {
        _currentPiece.x += dx;
        _currentPiece.y += dy;
        renderDynamic();

        // Lock Reset: If piece is at the bottom, lateral movement resets/extends lock delay
        if (_isGrounded && _lockMoves < 15) {
            _lockTimer = 0;
            _lockMoves++;
        }
    }
}

void GameEngine::dropPiece() {
    if (_state != STATE_PLAYING) return;

    int8_t gx = 0, gy = 0;
    getGravityVector(gx, gy);

    if (!collides(gx, gy, _currentPiece.matrix, _currentPiece.size, _currentPiece.x, _currentPiece.y)) {
        _currentPiece.x += gx;
        _currentPiece.y += gy;
        renderDynamic();
    } else {
        // Landed on floor or blocks: enter lock delay grace window
        _isGrounded = true;
    }
}

void GameEngine::hardDrop() {
    if (_state != STATE_PLAYING) return;

    int8_t gx = 0, gy = 0;
    getGravityVector(gx, gy);

    while (!collides(gx, gy, _currentPiece.matrix, _currentPiece.size, _currentPiece.x, _currentPiece.y)) {
        _currentPiece.x += gx;
        _currentPiece.y += gy;
    }
    mergePiece();
    clearLines();

    // Check if 3x3 center box ceiling was breached after piece lock
    if (isCenterBoxBreached()) {
        _state = STATE_GAME_OVER;
        if (_score > _highScore) {
            _highScore = _score;
            SDConfig.saveHighScore(_highScore);
        }
        drawGameOverScreen();
        return;
    }

    spawnPiece();
    if (_state == STATE_PLAYING) {
        renderDynamic();
    }
    _dropCounter = 0;
    _lockTimer = 0;
    _lockMoves = 0;
    _isGrounded = false;
}

void GameEngine::setGravityDirection(GravityDirection dir) {
    if (_state != STATE_PLAYING || _gravityDir == dir) return;
    
    _gravityDir = dir;
    _lockTimer = 0; // Reset lock timer on orientation change
    renderDynamic();
}

void GameEngine::getGhostPos(int8_t &ghostX, int8_t &ghostY) {
    ghostX = _currentPiece.x;
    ghostY = _currentPiece.y;

    int8_t gx = 0, gy = 0;
    getGravityVector(gx, gy);

    while (!collides(gx, gy, _currentPiece.matrix, _currentPiece.size, ghostX, ghostY)) {
        ghostX += gx;
        ghostY += gy;
    }
}

void GameEngine::mergePiece() {
    for (int r = 0; r < _currentPiece.size; r++) {
        for (int c = 0; c < _currentPiece.size; c++) {
            if (_currentPiece.matrix[r][c] != 0) {
                int8_t boardY = _currentPiece.y + r;
                int8_t boardX = _currentPiece.x + c;
                if (boardY >= 0 && boardY < GRID_ROWS && boardX >= 0 && boardX < GRID_COLS) {
                    _board[boardY][boardX] = _currentPiece.matrix[r][c];
                    int16_t px = PLAYFIELD_X + (boardX * BLOCK_SIZE);
                    int16_t py = PLAYFIELD_Y + (boardY * BLOCK_SIZE);
                    Display.drawBeveledBlock(px, py, BLOCK_SIZE, PIECE_COLORS[_board[boardY][boardX]], false);
                }
            }
        }
    }
    _hasPrevPiece = false;
    _isGrounded = false;
    _lockTimer = 0;
    _lockMoves = 0;
}

void GameEngine::redrawBoard() {
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            eraseTile(r, c);
        }
    }
}

void GameEngine::clearLines() {
    int totalCleared = 0;

    // 1. Bottom Well (rows 12 down to 7 across cols 3..9)
    for (int r = 12; r >= 7; r--) {
        bool full = true;
        for (int c = 3; c <= 9; c++) {
            if (_board[r][c] <= 0) { full = false; break; }
        }
        if (full) {
            totalCleared++;
            for (int y = r; y >= 3; y--) {
                for (int c = 3; c <= 9; c++) {
                    _board[y][c] = (y > 0) ? _board[y - 1][c] : 0;
                }
            }
            r++;
        }
    }

    // 2. Top Well (rows 0 up to 5 across cols 3..9)
    for (int r = 0; r <= 5; r++) {
        bool full = true;
        for (int c = 3; c <= 9; c++) {
            if (_board[r][c] <= 0) { full = false; break; }
        }
        if (full) {
            totalCleared++;
            for (int y = r; y <= 9; y++) {
                for (int c = 3; c <= 9; c++) {
                    _board[y][c] = (y < 12) ? _board[y + 1][c] : 0;
                }
            }
            r--;
        }
    }

    // 3. Left Well (cols 0 to 5 across rows 3..9)
    for (int c = 0; c <= 5; c++) {
        bool full = true;
        for (int r = 3; r <= 9; r++) {
            if (_board[r][c] <= 0) { full = false; break; }
        }
        if (full) {
            totalCleared++;
            for (int x = c; x <= 9; x++) {
                for (int r = 3; r <= 9; r++) {
                    _board[r][x] = (x < 12) ? _board[r][x + 1] : 0;
                }
            }
            c--;
        }
    }

    // 4. Right Well (cols 12 down to 7 across rows 3..9)
    for (int c = 12; c >= 7; c--) {
        bool full = true;
        for (int r = 3; r <= 9; r++) {
            if (_board[r][c] <= 0) { full = false; break; }
        }
        if (full) {
            totalCleared++;
            for (int x = c; x >= 3; x--) {
                for (int r = 3; r <= 9; r++) {
                    _board[r][x] = (x > 0) ? _board[r][x - 1] : 0;
                }
            }
            c++;
        }
    }

    if (totalCleared > 0) {
        const uint32_t multi[5] = {0, 100, 300, 500, 800};
        uint32_t base = (totalCleared <= 4) ? multi[totalCleared] : (totalCleared * 200);

        // Difficulty Score Multipliers
        float factor = 1.0f;
        if (_difficulty == DIFF_MEDIUM) factor = 1.5f;
        else if (_difficulty == DIFF_HARD) factor = 2.5f;

        _score += (uint32_t)(base * factor);
        drawHUD();
        redrawBoard();
    }
}

uint32_t GameEngine::getCurrentSpeed() {
    if (_difficulty == DIFF_EASY) {
        return max(400, (int)(900 - ((_score / 500) * 30)));
    } else if (_difficulty == DIFF_HARD) {
        return max(120, (int)(400 - ((_score / 500) * 50)));
    } else { // MEDIUM
        return max(250, (int)(650 - ((_score / 500) * 40)));
    }
}

void GameEngine::drawDifficultyButtons() {
    // Button 1: EASY (Y = 95..145)
    bool isEasy = (_difficulty == DIFF_EASY);
    uint16_t easyBg = isEasy ? COLOR_TEXT_GREEN : COLOR_BG;
    uint16_t easyFg = isEasy ? COLOR_BG : COLOR_TEXT_GREEN;
    Display.fillRect(30, 95, SCREEN_WIDTH - 60, 48, easyBg);
    Display.drawRect(30, 95, SCREEN_WIDTH - 60, 48, COLOR_TEXT_GREEN);
    Display.drawRect(31, 96, SCREEN_WIDTH - 62, 46, COLOR_TEXT_GREEN);
    Display.drawCenteredString(108, "EASY  (900ms)", easyFg, easyBg, 2);

    // Button 2: MEDIUM (Y = 155..205)
    bool isMed = (_difficulty == DIFF_MEDIUM);
    uint16_t medBg = isMed ? COLOR_TEXT_AMBER : COLOR_BG;
    uint16_t medFg = isMed ? COLOR_BG : COLOR_TEXT_AMBER;
    Display.fillRect(30, 155, SCREEN_WIDTH - 60, 48, medBg);
    Display.drawRect(30, 155, SCREEN_WIDTH - 60, 48, COLOR_TEXT_AMBER);
    Display.drawRect(31, 156, SCREEN_WIDTH - 62, 46, COLOR_TEXT_AMBER);
    Display.drawCenteredString(168, "MEDIUM  (650ms)", medFg, medBg, 2);

    // Button 3: HARD (Y = 215..265)
    bool isHard = (_difficulty == DIFF_HARD);
    uint16_t hardBg = isHard ? COLOR_TEXT_RED : COLOR_BG;
    uint16_t hardFg = isHard ? COLOR_BG : COLOR_TEXT_RED;
    Display.fillRect(30, 215, SCREEN_WIDTH - 60, 48, hardBg);
    Display.drawRect(30, 215, SCREEN_WIDTH - 60, 48, COLOR_TEXT_RED);
    Display.drawRect(31, 216, SCREEN_WIDTH - 62, 46, COLOR_TEXT_RED);
    Display.drawCenteredString(228, "HARD  (400ms)", hardFg, hardBg, 2);
}

void GameEngine::setDifficulty(Difficulty diff) {
    if (_difficulty == diff) return;
    _difficulty = diff;
    drawDifficultyButtons();
}

void GameEngine::handleMenuTouch(int16_t x, int16_t y) {
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

void GameEngine::showStartScreen() {
    _state = STATE_START_MENU;
    drawStartScreen();
}

void GameEngine::drawCoreBox() {
    int16_t startX = PLAYFIELD_X;
    int16_t startY = PLAYFIELD_Y;
    int16_t coreX = startX + (5 * BLOCK_SIZE);
    int16_t coreY = startY + (5 * BLOCK_SIZE);
    int16_t coreSize = 3 * BLOCK_SIZE; // 78 px

    // 1. Fill solid core background plate
    Display.fillRect(coreX, coreY, coreSize, coreSize, COLOR_CORE_FILL);

    // 2. Interior 3x3 grid lines
    for (int r = 5; r <= 7; r++) {
        for (int c = 5; c <= 7; c++) {
            int16_t px = startX + (c * BLOCK_SIZE);
            int16_t py = startY + (r * BLOCK_SIZE);
            Display.drawRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_CORE_GRID);
        }
    }

    // 3. Crisp Continuous 2px Neon Cyan Border
    Display.drawRect(coreX - 1, coreY - 1, coreSize + 2, coreSize + 2, COLOR_CORE_BORDER);
    Display.drawRect(coreX, coreY, coreSize, coreSize, COLOR_CORE_BORDER);
}

void GameEngine::drawCountdownDigit(const char *digit, uint16_t color) {
    int16_t coreX = PLAYFIELD_X + (5 * BLOCK_SIZE);
    int16_t coreY = PLAYFIELD_Y + (5 * BLOCK_SIZE);
    int16_t coreSize = 3 * BLOCK_SIZE;

    // Fill interior of the solid 3x3 core box
    Display.fillRect(coreX + 2, coreY + 2, coreSize - 4, coreSize - 4, COLOR_CORE_FILL);

    // Draw the glowing countdown digit centered right inside the core box (Y = 204)
    Display.drawCenteredString(204, digit, color, COLOR_CORE_FILL, 5);
}

void GameEngine::clearCountdownBox() {
    drawCoreBox();
}

void GameEngine::resetGame() {
    initEmptyBoard();
    _score = 0;
    _dropCounter = 0;
    _lockTimer = 0;
    _lockMoves = 0;
    _isGrounded = false;
    _gravityDir = GRAVITY_DOWN;
    _lastRotateTime = millis();
    _state = STATE_PLAYING;

    // 1. Draw the arena, HUD, and solid 3x3 center core box first
    Display.fillScreen(COLOR_BG);
    drawHUD();
    drawPlayfieldBackground();
    delay(300);

    // 2. Cinematic 3, 2, 1 Countdown centered inside the 3x3 core box
    drawCountdownDigit("3", COLOR_TEXT_CYAN);
    delay(600);

    drawCountdownDigit("2", COLOR_TEXT_AMBER);
    delay(600);

    drawCountdownDigit("1", COLOR_TEXT_GREEN);
    delay(600);

    // 3. Clear core box back to clean grid plate
    clearCountdownBox();
    delay(150);

    // 4. Spawn the first piece out of the solid core box
    spawnPiece();
    renderDynamic();
}

void GameEngine::update(uint32_t deltaMs) {
    if (_state != STATE_PLAYING) return;

    int8_t gx = 0, gy = 0;
    getGravityVector(gx, gy);

    bool onGround = collides(gx, gy, _currentPiece.matrix, _currentPiece.size, _currentPiece.x, _currentPiece.y);

    if (onGround) {
        _isGrounded = true;
        _lockTimer += deltaMs;

        // When lock delay expires while grounded, freeze piece permanently
        if (_lockTimer >= LOCK_DELAY_MS) {
            mergePiece();
            clearLines();

            // Check if 3x3 center box ceiling was breached after piece lock
            if (isCenterBoxBreached()) {
                _state = STATE_GAME_OVER;
                if (_score > _highScore) {
                    _highScore = _score;
                    SDConfig.saveHighScore(_highScore);
                }
                drawGameOverScreen();
                return;
            }

            spawnPiece();
            if (_state == STATE_PLAYING) {
                renderDynamic();
            }
            _dropCounter = 0;
            _lockTimer = 0;
            _lockMoves = 0;
            _isGrounded = false;
        }
    } else {
        // Piece is falling freely in the air
        _isGrounded = false;
        _lockTimer = 0;

        _dropCounter += deltaMs;
        uint32_t speed = getCurrentSpeed();
        if (_dropCounter >= speed) {
            dropPiece();
            _dropCounter = 0;
        }
    }
}

// -------------------------------------------------------------
// AMOLED Rendering Pipeline
// -------------------------------------------------------------

void GameEngine::eraseTile(int8_t r, int8_t c) {
    if (r < 0 || r >= GRID_ROWS || c < 0 || c >= GRID_COLS) return;
    int16_t startX = PLAYFIELD_X;
    int16_t startY = PLAYFIELD_Y;
    int16_t px = startX + (c * BLOCK_SIZE);
    int16_t py = startY + (r * BLOCK_SIZE);

    if (_board[r][c] > 0) {
        Display.drawBeveledBlock(px, py, BLOCK_SIZE, PIECE_COLORS[_board[r][c]], false);
    } else if (_board[r][c] == -1) {
        Display.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_WALL);
        Display.drawRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_WALL_GRID);
    } else if (r >= CORE_R_MIN && r <= CORE_R_MAX && c >= CORE_C_MIN && c <= CORE_C_MAX) {
        // Solid 3x3 Central Core chamber
        Display.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_CORE_FILL);
        Display.drawRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_CORE_GRID);
    } else {
        Display.fillRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_BG);
        Display.drawRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_GRID_LINE);
    }

    // Repair Cyan Wall Borders for 3x3 corners
    if (r == 3 && c < 3) Display.fillRect(px, py, BLOCK_SIZE, 2, COLOR_WALL_BORDER);
    if (r == 9 && c < 3) Display.fillRect(px, py + BLOCK_SIZE - 2, BLOCK_SIZE, 2, COLOR_WALL_BORDER);
    if (r == 3 && c > 9) Display.fillRect(px, py, BLOCK_SIZE, 2, COLOR_WALL_BORDER);
    if (r == 9 && c > 9) Display.fillRect(px, py + BLOCK_SIZE - 2, BLOCK_SIZE, 2, COLOR_WALL_BORDER);

    if (c == 3 && r < 3) Display.fillRect(px, py, 2, BLOCK_SIZE, COLOR_WALL_BORDER);
    if (c == 9 && r < 3) Display.fillRect(px + BLOCK_SIZE - 2, py, 2, BLOCK_SIZE, COLOR_WALL_BORDER);
    if (c == 3 && r > 9) Display.fillRect(px, py, 2, BLOCK_SIZE, COLOR_WALL_BORDER);
    if (c == 9 && r > 9) Display.fillRect(px + BLOCK_SIZE - 2, py, 2, BLOCK_SIZE, COLOR_WALL_BORDER);

    // If erasing inside or adjacent to the 3x3 center core box, redraw clean continuous border
    if (r >= 4 && r <= 8 && c >= 4 && c <= 8) {
        int16_t coreX = startX + (5 * BLOCK_SIZE);
        int16_t coreY = startY + (5 * BLOCK_SIZE);
        int16_t coreSize = 3 * BLOCK_SIZE;
        Display.drawRect(coreX - 1, coreY - 1, coreSize + 2, coreSize + 2, COLOR_CORE_BORDER);
        Display.drawRect(coreX, coreY, coreSize, coreSize, COLOR_CORE_BORDER);
    }
}

void GameEngine::drawPlayfieldBackground() {
    int16_t startX = PLAYFIELD_X;
    int16_t startY = PLAYFIELD_Y;
    int16_t totalSize = GRID_COLS * BLOCK_SIZE; // 338 px
    int16_t cornerPx = CORNER_SIZE * BLOCK_SIZE; // 3 * 26 = 78 px
    int16_t wMin = 3 * BLOCK_SIZE;  // 78 px
    int16_t wMax = 10 * BLOCK_SIZE; // 260 px

    // 1. Fill entire arena black
    Display.fillRect(startX, startY, totalSize, totalSize, COLOR_BG);

    // 2. Fill the 4 solid 3x3 corner quadrants (78x78 px each)
    Display.fillRect(startX, startY, cornerPx, cornerPx, COLOR_WALL);
    Display.fillRect(startX + wMax, startY, cornerPx, cornerPx, COLOR_WALL);
    Display.fillRect(startX, startY + wMax, cornerPx, cornerPx, COLOR_WALL);
    Display.fillRect(startX + wMax, startY + wMax, cornerPx, cornerPx, COLOR_WALL);

    // 3. Draw grid lines on all 13x13 cells
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            int16_t px = startX + (c * BLOCK_SIZE);
            int16_t py = startY + (r * BLOCK_SIZE);
            if (_board[r][c] == -1) {
                Display.drawRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_WALL_GRID);
            } else if (r >= CORE_R_MIN && r <= CORE_R_MAX && c >= CORE_C_MIN && c <= CORE_C_MAX) {
                // Drawn by drawCoreBox()
            } else {
                Display.drawRect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_GRID_LINE);
            }
        }
    }

    // 4. Inset 2px Cyan T-bar borders separating 3x3 corner walls from playable wells
    // Top-Left Corner
    Display.fillRect(startX + wMin - 1, startY, 2, wMin, COLOR_WALL_BORDER);
    Display.fillRect(startX, startY + wMin - 1, wMin, 2, COLOR_WALL_BORDER);

    // Top-Right Corner
    Display.fillRect(startX + wMax - 1, startY, 2, wMin, COLOR_WALL_BORDER);
    Display.fillRect(startX + wMax, startY + wMin - 1, wMin, 2, COLOR_WALL_BORDER);

    // Bottom-Left Corner
    Display.fillRect(startX, startY + wMax - 1, wMin, 2, COLOR_WALL_BORDER);
    Display.fillRect(startX + wMin - 1, startY + wMax, 2, wMin, COLOR_WALL_BORDER);

    // Bottom-Right Corner
    Display.fillRect(startX + wMax - 1, startY + wMax, 2, wMin, COLOR_WALL_BORDER);
    Display.fillRect(startX + wMax, startY + wMax - 1, wMin, 2, COLOR_WALL_BORDER);

    // 5. Draw Solid 3x3 Central Core Box (solid continuous rectangle)
    drawCoreBox();

    // 6. Outer Cyan Border enclosing the entire playfield (2px thick)
    Display.drawRect(startX - 2, startY - 2, totalSize + 4, totalSize + 4, COLOR_WALL_BORDER);
    Display.drawRect(startX - 1, startY - 1, totalSize + 2, totalSize + 2, COLOR_WALL_BORDER);
}

void GameEngine::drawHUD() {
    // Score on left
    char buf[32];
    snprintf(buf, sizeof(buf), "SCORE:%lu", _score);
    Display.drawString(10, 18, buf, COLOR_TEXT_CYAN, COLOR_BG, 2);

    // Difficulty badge in center
    if (_difficulty == DIFF_EASY) {
        Display.drawString(160, 18, "EASY", COLOR_TEXT_GREEN, COLOR_BG, 2);
    } else if (_difficulty == DIFF_HARD) {
        Display.drawString(160, 18, "HARD", COLOR_TEXT_RED, COLOR_BG, 2);
    } else {
        Display.drawString(160, 18, "MED", COLOR_TEXT_AMBER, COLOR_BG, 2);
    }

    // ROTATETRIS on right
    Display.drawString(SCREEN_WIDTH - 150, 18, "ROTATETRIS", COLOR_TEXT_CYAN, COLOR_BG, 1);
}

void GameEngine::renderAll() {
    drawHUD();
    drawPlayfieldBackground();

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (_board[r][c] > 0) {
                int16_t px = PLAYFIELD_X + (c * BLOCK_SIZE);
                int16_t py = PLAYFIELD_Y + (r * BLOCK_SIZE);
                Display.drawBeveledBlock(px, py, BLOCK_SIZE, PIECE_COLORS[_board[r][c]], false);
            }
        }
    }
    _hasPrevPiece = false;
    renderDynamic();
}

void GameEngine::renderDynamic() {
    if (_hasPrevPiece) {
        if (_prevGhostX >= 0 && _prevGhostY >= 0) {
            for (int r = 0; r < _prevPiece.size; r++) {
                for (int c = 0; c < _prevPiece.size; c++) {
                    if (_prevPiece.matrix[r][c] != 0) {
                        eraseTile(_prevGhostY + r, _prevGhostX + c);
                    }
                }
            }
        }

        for (int r = 0; r < _prevPiece.size; r++) {
            for (int c = 0; c < _prevPiece.size; c++) {
                if (_prevPiece.matrix[r][c] != 0) {
                    eraseTile(_prevPiece.y + r, _prevPiece.x + c);
                }
            }
        }
    }

    int8_t ghostX = 0, ghostY = 0;
    getGhostPos(ghostX, ghostY);

    // Draw Ghost (Translucent fill + border matching reference)
    for (int r = 0; r < _currentPiece.size; r++) {
        for (int c = 0; c < _currentPiece.size; c++) {
            if (_currentPiece.matrix[r][c] != 0) {
                int8_t gy = ghostY + r;
                int8_t gx = ghostX + c;
                if (gy >= 0 && gy < GRID_ROWS && gx >= 0 && gx < GRID_COLS && _board[gy][gx] == 0) {
                    int16_t px = PLAYFIELD_X + (gx * BLOCK_SIZE);
                    int16_t py = PLAYFIELD_Y + (gy * BLOCK_SIZE);
                    Display.drawBeveledBlock(px, py, BLOCK_SIZE, 0, true);
                }
            }
        }
    }

    // Draw Active Piece
    for (int r = 0; r < _currentPiece.size; r++) {
        for (int c = 0; c < _currentPiece.size; c++) {
            if (_currentPiece.matrix[r][c] != 0) {
                int8_t by = _currentPiece.y + r;
                int8_t bx = _currentPiece.x + c;
                if (by >= 0 && by < GRID_ROWS && bx >= 0 && bx < GRID_COLS) {
                    int16_t px = PLAYFIELD_X + (bx * BLOCK_SIZE);
                    int16_t py = PLAYFIELD_Y + (by * BLOCK_SIZE);
                    Display.drawBeveledBlock(px, py, BLOCK_SIZE, _currentPiece.color, false);
                }
            }
        }
    }

    _prevPiece = _currentPiece;
    _prevGhostX = ghostX;
    _prevGhostY = ghostY;
    _hasPrevPiece = true;
}

void GameEngine::drawStartScreen() {
    Display.fillScreen(COLOR_BG);

    // Outer Cyber Cyan Outline
    Display.drawRect(10, 10, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 20, COLOR_TEXT_CYAN);
    Display.drawRect(12, 12, SCREEN_WIDTH - 24, SCREEN_HEIGHT - 24, COLOR_TEXT_CYAN);

    // Title
    Display.drawCenteredString(30, "ROTATETRIS", COLOR_TEXT_CYAN, COLOR_BG, 3);
    Display.drawCenteredString(65, "SELECT DIFFICULTY", COLOR_TEXT_GRAY, COLOR_BG, 1);

    // Draw the 3 Difficulty Buttons
    drawDifficultyButtons();

    // High Score Record
    char hiBuf[32];
    snprintf(hiBuf, sizeof(hiBuf), "RECORD: %lu", _highScore);
    Display.drawCenteredString(290, hiBuf, COLOR_TEXT_WHITE, COLOR_BG, 2);

    // Launch Button (Y = 345..405)
    Display.fillRect(30, 345, SCREEN_WIDTH - 60, 58, COLOR_TEXT_CYAN);
    Display.drawCenteredString(363, "START GAME", COLOR_BG, COLOR_TEXT_CYAN, 3);
}

void GameEngine::drawGameOverScreen() {
    // Fill full screen so no background pieces or grid lines poke through
    Display.fillScreen(COLOR_BG);

    // Cyber Red Outline
    Display.drawRect(12, 12, SCREEN_WIDTH - 24, SCREEN_HEIGHT - 24, COLOR_TEXT_RED);
    Display.drawRect(14, 14, SCREEN_WIDTH - 28, SCREEN_HEIGHT - 28, COLOR_TEXT_RED);

    // Header
    Display.drawCenteredString(45, "SYSTEM HALT", COLOR_TEXT_RED, COLOR_BG, 3);
    Display.drawCenteredString(80, "CORE OVERFLOW DETECTED", COLOR_TEXT_AMBER, COLOR_BG, 1);

    // Red Divider
    Display.fillRect(30, 105, SCREEN_WIDTH - 60, 2, COLOR_TEXT_RED);

    // Final Score
    Display.drawCenteredString(130, "FINAL SCORE", COLOR_TEXT_GRAY, COLOR_BG, 1);
    char buf[32];
    snprintf(buf, sizeof(buf), "%lu", _score);
    Display.drawCenteredString(160, buf, COLOR_TEXT_WHITE, COLOR_BG, 4);

    // Best Record
    char hiBuf[32];
    snprintf(hiBuf, sizeof(hiBuf), "BEST RECORD: %lu", _highScore);
    Display.drawCenteredString(240, hiBuf, COLOR_TEXT_CYAN, COLOR_BG, 2);

    // Red Divider
    Display.fillRect(30, 280, SCREEN_WIDTH - 60, 2, COLOR_TEXT_RED);

    // Button 1: PLAY AGAIN (Y = 300..355)
    Display.fillRect(35, 300, SCREEN_WIDTH - 70, 50, COLOR_TEXT_RED);
    Display.drawCenteredString(316, "PLAY AGAIN", COLOR_BG, COLOR_TEXT_RED, 2);

    // Button 2: MAIN MENU (Y = 365..420)
    Display.fillRect(35, 365, SCREEN_WIDTH - 70, 50, COLOR_TEXT_CYAN);
    Display.drawCenteredString(381, "MAIN MENU", COLOR_BG, COLOR_TEXT_CYAN, 2);
}
