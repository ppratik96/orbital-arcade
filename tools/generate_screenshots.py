#!/usr/bin/env python3
import os
from PIL import Image, ImageDraw, ImageFont

# Screen Dimensions
WIDTH = 368
HEIGHT = 448

# Color Palette (RGB)
COLOR_BG = (0, 0, 0)
COLOR_CYAN = (0, 240, 255)
COLOR_GREEN = (0, 255, 0)
COLOR_AMBER = (255, 190, 0)
COLOR_RED = (255, 30, 30)
COLOR_WHITE = (255, 255, 255)
COLOR_GRAY = (140, 140, 140)
COLOR_DARK_GRAY = (40, 45, 55)

# Card Fills
COLOR_CARD_TETRIS = (8, 20, 45)
COLOR_CARD_SNAKE = (1, 40, 20)

# Tetris Arena
COLOR_WALL = (8, 12, 24)
COLOR_WALL_GRID = (16, 28, 48)
COLOR_GRID_LINE = (30, 35, 45)
COLOR_CORE_FILL = (24, 64, 96)
COLOR_CORE_GRID = (34, 40, 142)

# Pieces
PIECE_CYAN = (0, 240, 255)
PIECE_BLUE = (0, 100, 255)
PIECE_ORANGE = (255, 140, 0)
PIECE_YELLOW = (255, 220, 0)
PIECE_GREEN = (0, 230, 0)
PIECE_PURPLE = (160, 0, 200)
PIECE_RED = (255, 30, 30)

def get_font(size):
    try:
        return ImageFont.truetype("/System/Library/Fonts/SFNSMono.ttf", size)
    except:
        try:
            return ImageFont.truetype("/System/Library/Fonts/Menlo.ttc", size)
        except:
            return ImageFont.load_default()

def draw_beveled_block(draw, x, y, size, color, is_ghost=False):
    if is_ghost:
        draw.rectangle([x + 1, y + 1, x + size - 2, y + size - 2], fill=(32, 34, 40), outline=(88, 92, 104), width=1)
        return
    # Outer black
    draw.rectangle([x, y, x + size - 1, y + size - 1], outline=(0, 0, 0), width=1)
    # Fill
    draw.rectangle([x + 1, y + 1, x + size - 2, y + size - 2], fill=color)
    # Highlight (Top & Left)
    draw.line([x + 1, y + 1, x + size - 2, y + 1], fill=(255, 255, 255), width=1)
    draw.line([x + 1, y + 1, x + 1, y + size - 2], fill=(255, 255, 255), width=1)
    # Shadow (Bottom & Right)
    draw.line([x + 1, y + size - 2, x + size - 2, y + size - 2], fill=(0, 0, 0), width=1)
    draw.line([x + size - 2, y + 1, x + size - 2, y + size - 2], fill=(0, 0, 0), width=1)

def generate_homescreen(filename="assets/homescreen.png"):
    img = Image.new("RGB", (WIDTH, HEIGHT), COLOR_BG)
    draw = ImageDraw.Draw(img)

    font_large = get_font(26)
    font_med = get_font(18)
    font_sub = get_font(14)
    font_small = get_font(11)

    # Outer Frame
    draw.rectangle([8, 8, WIDTH - 9, HEIGHT - 9], outline=(26, 16, 40), width=2)
    draw.rectangle([10, 10, WIDTH - 11, HEIGHT - 11], outline=COLOR_AMBER, width=2)

    # Header
    title = "ORBITAL ARCADE"
    bbox = draw.textbbox((0, 0), title, font=font_large)
    w = bbox[2] - bbox[0]
    draw.text(((WIDTH - w) // 2, 16), title, fill=COLOR_AMBER, font=font_large)
    draw.line([30, 48, WIDTH - 30, 48], fill=COLOR_AMBER, width=2)
    
    sub = "-- SELECT CARTRIDGE --"
    bbox = draw.textbbox((0, 0), sub, font=font_small)
    draw.text(((WIDTH - (bbox[2] - bbox[0])) // 2, 52), sub, fill=COLOR_GRAY, font=font_small)

    # Card 1: ROTATETRIS (Y = 70, H = 160)
    c1y = 70
    draw.rectangle([12, c1y, WIDTH - 13, c1y + 160], fill=COLOR_CARD_TETRIS, outline=COLOR_CYAN, width=2)

    # Mini Icon (Left)
    draw_beveled_block(draw, 26, c1y + 20, 9, PIECE_CYAN)
    draw_beveled_block(draw, 17, c1y + 29, 9, PIECE_CYAN)
    draw_beveled_block(draw, 26, c1y + 29, 9, PIECE_CYAN)
    draw_beveled_block(draw, 35, c1y + 29, 9, PIECE_CYAN)

    draw_beveled_block(draw, 35, c1y + 42, 9, PIECE_ORANGE)
    draw_beveled_block(draw, 35, c1y + 51, 9, PIECE_ORANGE)
    draw_beveled_block(draw, 44, c1y + 51, 9, PIECE_ORANGE)

    # Text
    draw.text((64, c1y + 14), "ROTATETRIS", fill=COLOR_CYAN, font=font_large)
    draw.text((64, c1y + 48), "4-Way Gravity", fill=COLOR_WHITE, font=font_med)
    draw.text((64, c1y + 74), "BEST: 12500", fill=COLOR_AMBER, font=font_med)

    # Right Button
    btn1x = 262
    btn1y = c1y + 14
    draw.rectangle([btn1x, btn1y, btn1x + 82, btn1y + 132], fill=COLOR_CYAN, outline=COLOR_WHITE, width=2)
    draw.text((btn1x + 18, btn1y + 36), "PLAY", fill=(0, 0, 0), font=font_med)
    draw.text((btn1x + 32, btn1y + 64), ">", fill=(0, 0, 0), font=font_large)

    # Card 2: ROSNAKE (Y = 240, H = 160)
    c2y = 240
    draw.rectangle([12, c2y, WIDTH - 13, c2y + 160], fill=COLOR_CARD_SNAKE, outline=COLOR_GREEN, width=2)

    # Mini Snake Icon (Left)
    draw_beveled_block(draw, 46, c2y + 22, 9, COLOR_AMBER) # Food
    draw_beveled_block(draw, 35, c2y + 22, 9, COLOR_GREEN) # Head
    draw.rectangle([41, c2y + 24, 43, c2y + 26], fill=(0, 0, 0))
    draw.rectangle([41, c2y + 28, 43, c2y + 30], fill=(0, 0, 0))

    draw_beveled_block(draw, 24, c2y + 22, 9, (0, 180, 0))
    draw_beveled_block(draw, 15, c2y + 22, 9, (0, 180, 0))
    draw_beveled_block(draw, 15, c2y + 33, 9, (0, 180, 0))
    draw_beveled_block(draw, 15, c2y + 44, 9, (0, 180, 0))
    draw_beveled_block(draw, 26, c2y + 44, 9, (0, 180, 0))

    # Text
    draw.text((64, c2y + 14), "ROSNAKE", fill=COLOR_GREEN, font=font_large)
    draw.text((64, c2y + 48), "Tilt Steered", fill=COLOR_WHITE, font=font_med)
    draw.text((64, c2y + 74), "BEST: 8400", fill=COLOR_AMBER, font=font_med)

    # Right Button
    btn2x = 262
    btn2y = c2y + 14
    draw.rectangle([btn2x, btn2y, btn2x + 82, btn2y + 132], fill=COLOR_GREEN, outline=COLOR_WHITE, width=2)
    draw.text((btn2x + 18, btn2y + 36), "PLAY", fill=(0, 0, 0), font=font_med)
    draw.text((btn2x + 32, btn2y + 64), ">", fill=(0, 0, 0), font=font_large)

    os.makedirs(os.path.dirname(os.path.abspath(filename)), exist_ok=True)
    img.save(filename)
    print(f"Saved {filename}")

def generate_rotatetris(filename="assets/rotatetris.png"):
    img = Image.new("RGB", (WIDTH, HEIGHT), COLOR_BG)
    draw = ImageDraw.Draw(img)

    font_hud = get_font(18)
    font_small = get_font(12)

    # HUD
    draw.text((16, 12), "SCORE: 3400", fill=COLOR_CYAN, font=font_hud)
    draw.text((WIDTH - 130, 12), "LINES: 04", fill=COLOR_AMBER, font=font_hud)
    draw.line([15, 38, WIDTH - 15, 38], fill=COLOR_DARK_GRAY, width=1)

    # Arena Layout
    px_x = 15
    px_y = 55
    block_sz = 26
    total_sz = 13 * block_sz # 338

    # Background
    draw.rectangle([px_x, px_y, px_x + total_sz, px_y + total_sz], fill=COLOR_BG)

    # 4 Solid Corner 3x3 Walls (Navy)
    corner_sz = 3 * block_sz # 78
    w_max = 10 * block_sz # 260
    draw.rectangle([px_x, px_y, px_x + corner_sz, px_y + corner_sz], fill=COLOR_WALL)
    draw.rectangle([px_x + w_max, px_y, px_x + total_sz, px_y + corner_sz], fill=COLOR_WALL)
    draw.rectangle([px_x, px_y + w_max, px_x + corner_sz, px_y + total_sz], fill=COLOR_WALL)
    draw.rectangle([px_x + w_max, px_y + w_max, px_x + total_sz, px_y + total_sz], fill=COLOR_WALL)

    # Grid Lines
    for r in range(13):
        for c in range(13):
            x = px_x + c * block_sz
            y = px_y + r * block_sz
            if (r < 3 and c < 3) or (r < 3 and c > 9) or (r > 9 and c < 3) or (r > 9 and c > 9):
                draw.rectangle([x, y, x + block_sz, y + block_sz], outline=COLOR_WALL_GRID, width=1)
            elif 5 <= r <= 7 and 5 <= c <= 7:
                pass # core box handled below
            else:
                draw.rectangle([x, y, x + block_sz, y + block_sz], outline=COLOR_GRID_LINE, width=1)

    # Solid 3x3 Center Box
    core_x = px_x + 5 * block_sz
    core_y = px_y + 5 * block_sz
    core_sz = 3 * block_sz
    draw.rectangle([core_x, core_y, core_x + core_sz, core_y + core_sz], fill=COLOR_CORE_FILL)
    for r in range(5, 8):
        for c in range(5, 8):
            x = px_x + c * block_sz
            y = px_y + r * block_sz
            draw.rectangle([x, y, x + block_sz, y + block_sz], outline=COLOR_CORE_GRID, width=1)
    draw.rectangle([core_x - 1, core_y - 1, core_x + core_sz + 1, core_y + core_sz + 1], outline=COLOR_CYAN, width=3)

    # Corner Wall Cyan Separators
    draw.line([px_x + corner_sz, px_y, px_x + corner_sz, px_y + corner_sz], fill=COLOR_CYAN, width=3)
    draw.line([px_x, px_y + corner_sz, px_x + corner_sz, px_y + corner_sz], fill=COLOR_CYAN, width=3)

    draw.line([px_x + w_max, px_y, px_x + w_max, px_y + corner_sz], fill=COLOR_CYAN, width=3)
    draw.line([px_x + w_max, px_y + corner_sz, px_x + total_sz, px_y + corner_sz], fill=COLOR_CYAN, width=3)

    draw.line([px_x, px_y + w_max, px_x + corner_sz, px_y + w_max], fill=COLOR_CYAN, width=3)
    draw.line([px_x + corner_sz, px_y + w_max, px_x + corner_sz, px_y + total_sz], fill=COLOR_CYAN, width=3)

    draw.line([px_x + w_max, px_y + w_max, px_x + total_sz, px_y + w_max], fill=COLOR_CYAN, width=3)
    draw.line([px_x + w_max, px_y + w_max, px_x + w_max, px_y + total_sz], fill=COLOR_CYAN, width=3)

    # Outer Arena Border
    draw.rectangle([px_x - 2, px_y - 2, px_x + total_sz + 2, px_y + total_sz + 2], outline=COLOR_CYAN, width=3)

    # Stacked Pieces at bottom well
    for c in range(3, 10):
        if c != 6:
            draw_beveled_block(draw, px_x + c * block_sz, px_y + 12 * block_sz, block_sz, PIECE_BLUE)
        if c in (4, 5, 7, 8):
            draw_beveled_block(draw, px_x + c * block_sz, px_y + 11 * block_sz, block_sz, PIECE_ORANGE)

    # Ghost Piece
    draw_beveled_block(draw, px_x + 5 * block_sz, px_y + 10 * block_sz, block_sz, PIECE_CYAN, is_ghost=True)
    draw_beveled_block(draw, px_x + 6 * block_sz, px_y + 10 * block_sz, block_sz, PIECE_CYAN, is_ghost=True)
    draw_beveled_block(draw, px_x + 6 * block_sz, px_y + 11 * block_sz, block_sz, PIECE_CYAN, is_ghost=True)
    draw_beveled_block(draw, px_x + 6 * block_sz, px_y + 12 * block_sz, block_sz, PIECE_CYAN, is_ghost=True)

    # Active Falling T-Piece emerging from Core
    draw_beveled_block(draw, px_x + 5 * block_sz, px_y + 8 * block_sz, block_sz, PIECE_PURPLE)
    draw_beveled_block(draw, px_x + 6 * block_sz, px_y + 8 * block_sz, block_sz, PIECE_PURPLE)
    draw_beveled_block(draw, px_x + 7 * block_sz, px_y + 8 * block_sz, block_sz, PIECE_PURPLE)
    draw_beveled_block(draw, px_x + 6 * block_sz, px_y + 9 * block_sz, block_sz, PIECE_PURPLE)

    # Sub status
    draw.text((15, HEIGHT - 30), "GRAVITY: DOWN (V)");
    draw.text((15, HEIGHT - 30), "GRAVITY: DOWN (V)", fill=COLOR_GRAY, font=font_small)
    draw.text((WIDTH - 120, HEIGHT - 30), "DIFF: MEDIUM", fill=COLOR_AMBER, font=font_small)

    os.makedirs(os.path.dirname(os.path.abspath(filename)), exist_ok=True)
    img.save(filename)
    print(f"Saved {filename}")

def generate_rosnake(filename="assets/rosnake.png"):
    img = Image.new("RGB", (WIDTH, HEIGHT), COLOR_BG)
    draw = ImageDraw.Draw(img)

    font_hud = get_font(18)
    font_small = get_font(12)

    # HUD
    draw.text((16, 12), "SCORE: 1200", fill=COLOR_GREEN, font=font_hud)
    draw.text((WIDTH - 130, 12), "LENGTH: 16", fill=COLOR_AMBER, font=font_hud)
    draw.line([15, 38, WIDTH - 15, 38], fill=COLOR_DARK_GRAY, width=1)

    px_x = 15
    px_y = 55
    block_sz = 26
    total_sz = 13 * block_sz

    # Grid
    for r in range(13):
        for c in range(13):
            x = px_x + c * block_sz
            y = px_y + r * block_sz
            draw.rectangle([x, y, x + block_sz, y + block_sz], fill=COLOR_BG, outline=(25, 40, 30), width=1)

    # Outer Green Border
    draw.rectangle([px_x - 2, px_y - 2, px_x + total_sz + 2, px_y + total_sz + 2], outline=COLOR_GREEN, width=3)

    # Food Orbs
    draw_beveled_block(draw, px_x + 9 * block_sz, px_y + 4 * block_sz, block_sz, COLOR_AMBER)
    # Bonus Magenta Core
    draw_beveled_block(draw, px_x + 3 * block_sz, px_y + 9 * block_sz, block_sz, (255, 0, 220))

    # Snake Segments
    snake_coords = [
        (8, 4), (7, 4), (6, 4), (5, 4), (5, 5), (5, 6), (5, 7), (6, 7), (7, 7), (8, 7), (8, 8), (8, 9), (7, 9)
    ]
    for i, (c, r) in enumerate(snake_coords):
        if i == 0:
            # Head with eye dots
            hx = px_x + c * block_sz
            hy = px_y + r * block_sz
            draw_beveled_block(draw, hx, hy, block_sz, COLOR_GREEN)
            draw.rectangle([hx + block_sz - 9, hy + 5, hx + block_sz - 5, hy + 9], fill=(0, 0, 0))
            draw.rectangle([hx + block_sz - 9, hy + block_sz - 9, hx + block_sz - 5, hy + block_sz - 5], fill=(0, 0, 0))
        else:
            color = (0, 200, 0) if i % 2 == 0 else (0, 160, 0)
            draw_beveled_block(draw, px_x + c * block_sz, px_y + r * block_sz, block_sz, color)

    # Sub status
    draw.text((15, HEIGHT - 30), "GYRO: STEERING ON", fill=COLOR_GRAY, font=font_small)
    draw.text((WIDTH - 120, HEIGHT - 30), "DIFF: MEDIUM", fill=COLOR_AMBER, font=font_small)

    os.makedirs(os.path.dirname(os.path.abspath(filename)), exist_ok=True)
    img.save(filename)
    print(f"Saved {filename}")

if __name__ == "__main__":
    generate_homescreen("assets/homescreen.png")
    generate_rotatetris("assets/rotatetris.png")
    generate_rosnake("assets/rosnake.png")
