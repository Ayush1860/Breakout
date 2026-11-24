#pragma once

#include "raylib.h"

// Every tunable number in the game lives here.
namespace cfg {

// Window
constexpr int  SCREEN_W = 800;
constexpr int  SCREEN_H = 600;
constexpr char WINDOW_TITLE[] = "Breakout";

// Timing
constexpr float FIXED_DT       = 1.0f / 120.0f; // physics step (s)
constexpr float MAX_FRAME_TIME = 0.25f;         // clamp so a long stall can't trigger hundreds of steps

// Paddle
constexpr float PADDLE_W     = 100.0f;
constexpr float PADDLE_H     = 16.0f;
constexpr float PADDLE_Y     = 560.0f;  // top edge
constexpr float PADDLE_SPEED = 500.0f;  // px/s

// Ball
constexpr float BALL_RADIUS        = 8.0f;
constexpr float BALL_START_SPEED   = 350.0f;          // px/s
constexpr float BALL_MAX_SPEED     = 650.0f;          // px/s
constexpr float MIN_VERTICAL_RATIO = 0.25f;           // |vy| >= ratio * speed
constexpr float MAX_BOUNCE_ANGLE   = 60.0f * DEG2RAD; // from vertical, at the paddle's edge
constexpr float LAUNCH_MAX_ANGLE   = 10.0f * DEG2RAD; // random spread when launching

// No tunnelling: if the ball moves less than its radius per step, its centre
// can't cross a brick edge without the circle overlapping that brick first.
static_assert(BALL_MAX_SPEED * FIXED_DT < BALL_RADIUS, "ball can skip through bricks");

// Bricks
constexpr int   BRICK_COLS   = 10;
constexpr int   BRICK_ROWS   = 6;
constexpr float BRICK_W      = 70.0f;
constexpr float BRICK_H      = 24.0f;
constexpr float BRICK_GAP    = 6.0f;
constexpr float BRICK_TOP    = 80.0f;
constexpr float BRICK_LEFT   = (SCREEN_W - (BRICK_COLS * BRICK_W + (BRICK_COLS - 1) * BRICK_GAP)) / 2.0f;
constexpr int   BRICK_MAX_HP = 3;

// Scoring
constexpr int SCORE_PER_HP = 10;  // a brick is worth SCORE_PER_HP * maxHp

// HUD
constexpr int HUD_FONT_SIZE = 20;
constexpr int HUD_MARGIN    = 16;

// Palette
constexpr Color COL_BACKGROUND      {0x2C, 0x2C, 0x2A, 0xFF};
constexpr Color COL_TEXT            {0xF1, 0xEF, 0xE8, 0xFF};
constexpr Color COL_PADDLE          {0x85, 0xB7, 0xEB, 0xFF};
constexpr Color COL_BALL            {0xF1, 0xEF, 0xE8, 0xFF};
constexpr Color COL_BRICK_3         {0xD8, 0x5A, 0x30, 0xFF};
constexpr Color COL_BRICK_3_DAMAGED {0x99, 0x3C, 0x1D, 0xFF};
constexpr Color COL_BRICK_2         {0xEF, 0x9F, 0x27, 0xFF};
constexpr Color COL_BRICK_2_DAMAGED {0x85, 0x4F, 0x0B, 0xFF};
constexpr Color COL_BRICK_1         {0x1D, 0x9E, 0x75, 0xFF};

} // namespace cfg
