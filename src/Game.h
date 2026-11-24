#pragma once

#include "Ball.h"
#include "Brick.h"
#include "Paddle.h"
#include "Particles.h"

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

    // Pauses if a game is in progress (e.g. the window lost focus).
    void pause();

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

    // Effects
    void updateEffects(float dt);
    void addShake(float amount);

    // Drawing
    void drawWorld() const;
    void drawBricks() const;
    void drawHud() const;
    void drawControlsHint() const;
    void drawOverlay(const char* title, Color titleColor, const char* subtitle) const;

    GameState state_ = GameState::Menu;
    Paddle paddle_;
    Ball ball_;
    std::vector<Brick> bricks_;
    Particles particles_;
    float shake_ = 0.0f;        // current shake strength (px)
    Vector2 shakeOffset_{};     // this step's camera offset
    int level_ = 0;      // 0-based
    int lives_ = 0;
    int score_ = 0;
    int bestScore_ = 0;
    float paddleDir_ = 0.0f;
    std::mt19937 rng_;
};
