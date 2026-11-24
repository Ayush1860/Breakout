#pragma once

#include "Ball.h"
#include "Brick.h"
#include "Paddle.h"

#include <random>
#include <vector>

enum class GameState { Menu, Playing, Paused, LevelClear, GameOver, Win };

class Game {
public:
    Game();

    // Called once per rendered frame, before the fixed steps.
    void handleInput();
    // Called zero or more times per frame with cfg::FIXED_DT.
    void update(float dt);
    void draw() const;

private:
    // Flow
    void newGame();
    void startLevel(int index);
    void updatePlaying(float dt);
    void loseLife();
    void finishLevel();

    // Ball and collisions
    void holdBallOnPaddle();
    void launchBall();
    void collideWalls();
    void collidePaddle();
    void collideBricks();
    void hitBrick(Brick& brick);
    bool allBricksCleared() const;

    // Drawing
    void drawBricks() const;
    void drawHud() const;
    void drawControlsHint() const;
    void drawOverlay(const char* title, Color titleColor, const char* subtitle) const;

    GameState state_ = GameState::Menu;
    Paddle paddle_;
    Ball ball_;
    std::vector<Brick> bricks_;
    int level_ = 0;      // 0-based
    int lives_ = 0;
    int score_ = 0;
    int bestScore_ = 0;
    float paddleDir_ = 0.0f;
    std::mt19937 rng_;
};
