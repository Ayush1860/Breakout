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
constexpr float MIN_VERTICAL_RATIO = 0.25f;           // |vy| >= ratio * speed
constexpr float MAX_BOUNCE_ANGLE   = 60.0f * DEG2RAD; // from vertical, at the paddle's edge
constexpr float LAUNCH_MAX_ANGLE   = 10.0f * DEG2RAD; // random spread when launching

// Palette
constexpr Color COL_BACKGROUND{0x2C, 0x2C, 0x2A, 0xFF};
constexpr Color COL_PADDLE    {0x85, 0xB7, 0xEB, 0xFF};
constexpr Color COL_BALL      {0xF1, 0xEF, 0xE8, 0xFF};

} // namespace cfg
