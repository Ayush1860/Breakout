#pragma once

#include "Ball.h"
#include "Paddle.h"

#include <random>

class Game {
public:
    Game();

    // Called once per rendered frame, before the fixed steps.
    void readInput();
    // Called zero or more times per frame with cfg::FIXED_DT.
    void update(float dt);
    void draw() const;

private:
    void holdBallOnPaddle();
    void launchBall();
    void collideWalls();
    void collidePaddle();

    Paddle paddle_;
    Ball ball_;
    std::mt19937 rng_;

    // Input latched by readInput() and consumed by update().
    float paddleDir_ = 0.0f;
    bool launchRequested_ = false;
};
