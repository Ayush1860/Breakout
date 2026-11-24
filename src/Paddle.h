#pragma once

#include "Entity.h"
#include "raylib.h"

class Paddle : public Entity {
public:
    Paddle();

    // -1 = left, 0 = still, +1 = right. Set by Game from the keyboard.
    void setDirection(float dir) { dir_ = dir; }

    void update(float dt) override;
    void draw() const override;

    void reset();
    Rectangle rect() const;
    float centerX() const;

private:
    float x_ = 0.0f;  // left edge; y is fixed
    float dir_ = 0.0f;
};
