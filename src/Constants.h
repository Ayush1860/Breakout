#pragma once

#include "raylib.h"

// Every tunable number in the game lives here.
namespace cfg {

// Window
constexpr int   SCREEN_W = 800;
constexpr int   SCREEN_H = 600;
constexpr char  WINDOW_TITLE[] = "Breakout";

// Palette
constexpr Color COL_BACKGROUND{0x2C, 0x2C, 0x2A, 0xFF};

} // namespace cfg
