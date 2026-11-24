#include "Game.h"

#include "Constants.h"

#include <algorithm>
#include <cmath>

Game::Game()
    : rng_(std::random_device{}())
{
    holdBallOnPaddle();
}

void Game::readInput()
{
    paddleDir_ = 0.0f;
    if (IsKeyDown(KEY_LEFT))  paddleDir_ -= 1.0f;
    if (IsKeyDown(KEY_RIGHT)) paddleDir_ += 1.0f;

    // A key press is a one-frame event, but a frame may run zero physics
    // steps. Latch it here so update() can't miss it.
    launchRequested_ = launchRequested_ || IsKeyPressed(KEY_SPACE);
}

void Game::update(float dt)
{
    paddle_.setDirection(paddleDir_);
    paddle_.update(dt);

    if (!ball_.isLaunched()) {
        holdBallOnPaddle();
        if (launchRequested_) {
            launchBall();
        }
    }
    launchRequested_ = false;

    ball_.update(dt);
    collideWalls();
    collidePaddle();

    if (ball_.position().y - cfg::BALL_RADIUS > cfg::SCREEN_H) {
        ball_.reset();
        holdBallOnPaddle();
    }
}

void Game::draw() const
{
    paddle_.draw();
    ball_.draw();
}

void Game::holdBallOnPaddle()
{
    ball_.setPosition({paddle_.centerX(), cfg::PADDLE_Y - cfg::BALL_RADIUS});
}

void Game::launchBall()
{
    std::uniform_real_distribution<float> spread(-cfg::LAUNCH_MAX_ANGLE, cfg::LAUNCH_MAX_ANGLE);
    ball_.launch(spread(rng_));
}

void Game::collideWalls()
{
    Vector2 pos = ball_.position();
    Vector2 vel = ball_.velocity();
    const float r = cfg::BALL_RADIUS;
    bool bounced = false;

    // Push the ball back inside and force the velocity's sign (instead of
    // negating it), so a ball still touching the wall can't flip back out.
    if (pos.x - r < 0.0f) {
        pos.x = r;
        vel.x = std::abs(vel.x);
        bounced = true;
    } else if (pos.x + r > cfg::SCREEN_W) {
        pos.x = cfg::SCREEN_W - r;
        vel.x = -std::abs(vel.x);
        bounced = true;
    }
    if (pos.y - r < 0.0f) {
        pos.y = r;
        vel.y = std::abs(vel.y);
        bounced = true;
    }

    if (bounced) {
        ball_.setPosition(pos);
        ball_.setDirection(vel);
    }
}

void Game::collidePaddle()
{
    if (ball_.velocity().y <= 0.0f) {
        return;  // only bounce a ball that is coming down
    }
    const Vector2 pos = ball_.position();
    if (!CheckCollisionCircleRec(pos, cfg::BALL_RADIUS, paddle_.rect())) {
        return;
    }

    // Hit position from -1 (left edge) to +1 (right edge) picks the angle:
    // centre sends the ball straight up, the edges at MAX_BOUNCE_ANGLE.
    const float halfW = cfg::PADDLE_W / 2.0f;
    const float offset = std::clamp((pos.x - paddle_.centerX()) / halfW, -1.0f, 1.0f);
    const float angle = offset * cfg::MAX_BOUNCE_ANGLE;

    ball_.setPosition({pos.x, cfg::PADDLE_Y - cfg::BALL_RADIUS});
    ball_.setDirection({std::sin(angle), -std::cos(angle)});
}
