#pragma once

#include "Ball.h"
#include "Brick.h"
#include "Paddle.h"

#include <random>
#include <vector>

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
    void collideBricks();
    void hitBrick(Brick& brick);
    void drawBricks() const;
    void drawHud() const;

    Paddle paddle_;
    Ball ball_;
    std::vector<Brick> bricks_;
    int score_ = 0;
    std::mt19937 rng_;

    // Input latched by readInput() and consumed by update().
    float paddleDir_ = 0.0f;
    bool launchRequested_ = false;
};
