#pragma once

#include "Constants.h"
#include "raylib.h"

// Plain data, stored by value in a std::vector. Broken bricks stay in the
// vector with hp == 0, so a level never reallocates or shifts mid-play.
struct Brick {
    Rectangle rect;
    int hp;
    int maxHp;

    bool alive() const { return hp > 0; }
};

// Colour comes from the brick's starting HP; it darkens once damaged.
inline Color brickColor(const Brick& b)
{
    const bool damaged = b.hp < b.maxHp;
    switch (b.maxHp) {
    case 3:  return damaged ? cfg::COL_BRICK_3_DAMAGED : cfg::COL_BRICK_3;
    case 2:  return damaged ? cfg::COL_BRICK_2_DAMAGED : cfg::COL_BRICK_2;
    default: return cfg::COL_BRICK_1;
    }
}
