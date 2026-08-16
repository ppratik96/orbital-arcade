#pragma once

#include <Arduino.h>

// ==========================================
// Hardware Pin Definitions (Waveshare 1.8" AMOLED)
// ==========================================

// Display QSPI Pins (Official Waveshare Pinout)
#define LCD_CS_PIN      12
#define LCD_SCK_PIN     11
#define LCD_D0_PIN      4
#define LCD_D1_PIN      5
#define LCD_D2_PIN      6
#define LCD_D3_PIN      7
#define LCD_RST_PIN     -1

// Screen Dimensions
#define SCREEN_WIDTH    368
#define SCREEN_HEIGHT   448

// I2C Bus
#define I2C_SDA_PIN     15
#define I2C_SCL_PIN     14
#define I2C_FREQ_HZ     400000

// I2C Addresses
#define IO_EXPANDER_ADDR 0x20
#define AXP2101_ADDR     0x34
#define FT3168_ADDR      0x38
#define CST820_ADDR      0x15
#define QMI8658_ADDR     0x6B
#define PCF85063_ADDR    0x51

// Touch Pins
#define TOUCH_INT_PIN   21

// Physical Buttons
#define BTN_BOOT_PIN    0   // Active LOW

enum Difficulty {
    DIFF_EASY = 0,
    DIFF_MEDIUM,
    DIFF_HARD
};

// ==========================================
// Game Constants & Sizing (26px Big Blocks + Slim 2x2 Corners)
// ==========================================
#define GRID_ROWS       13
#define GRID_COLS       13
#define BLOCK_SIZE      26  // 13 * 26 = 338 px (+44% bigger blocks!)

// Lock Delay (Slide & Tuck Time when at bottom)
#define LOCK_DELAY_MS   600 // 600ms grace period to tuck/slide pieces under overhangs

// Playfield offset on 368x448 screen
#define PLAYFIELD_X     15  // (368 - 338) / 2 = 15 px
#define PLAYFIELD_Y     55  // Top offset below HUD

// 3x3 Corner boundaries (7-block wide arcade wells!)
#define CORNER_SIZE     3   // 3x3 corner walls (78x78 px)
#define WELL_MIN        3   // Playable wells start at index 3 (cols/rows 3..9)
#define WELL_MAX        9   // Playable wells end at index 9 (7 blocks wide)

// 3x3 Central Spawn Box & Overflow Ceiling Boundary (rows 5..7, cols 5..7)
#define CORE_R_MIN      5
#define CORE_R_MAX      7
#define CORE_C_MIN      5
#define CORE_C_MAX      7

// Vibrant AMOLED Palette matching Reference Image (1:1)
#define COLOR_BG          0x0000  // Pure AMOLED Black
#define COLOR_WALL        0x0862  // Dark Navy for solid corner walls (#080c18)
#define COLOR_WALL_GRID   0x10E5  // Subtle Navy Grid in corners (#101c30)
#define COLOR_WALL_BORDER 0x07BF  // Vibrant Neon Cyan outline (#00f0ff)
#define COLOR_CORE_BORDER 0x07BF  // Neon Cyan for 3x3 central spawn box outline
#define COLOR_CORE_FILL   0x1A0C  // Solid Deep Steel Core Plate (#184060)
#define COLOR_CORE_GRID   0x228E  // Core plate grid line
#define COLOR_GRID_LINE   0x4A69  // Distinct Slate-Gray Grid Line (#484c58)
#define COLOR_GHOST_FILL  0x2124  // Translucent dark ghost fill (#202228)
#define COLOR_GHOST_EDGE  0x5ACB  // Ghost border (#585c68)
#define COLOR_TEXT_CYAN   0x07BF  // Neon Cyan text (#00f0ff)
#define COLOR_TEXT_GREEN  0x07E0  // Neon Green text (#00ff00)
#define COLOR_TEXT_WHITE  0xFFFF  // White text
#define COLOR_TEXT_RED    0xF800  // Red text
#define COLOR_TEXT_AMBER  0xFBE0  // Amber text
#define COLOR_TEXT_GRAY   0x8C71  // Gray text
