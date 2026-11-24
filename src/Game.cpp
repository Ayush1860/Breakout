#include "Game.h"

#include "Collision.h"
#include "Constants.h"
#include "Level.h"

#include <algorithm>
#include <cmath>

Game::Game()
    : bricks_(level::load(0))
    , rng_(std::random_device{}())
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
    collideBricks();
    collidePaddle();

    if (ball_.position().y - cfg::BALL_RADIUS > cfg::SCREEN_H) {
        ball_.reset();
        holdBallOnPaddle();
    }
}

void Game::draw() const
{
    drawBricks();
    paddle_.draw();
    ball_.draw();
    drawHud();
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
    if (!collision::circleRectOverlap(pos, cfg::BALL_RADIUS, paddle_.rect())) {
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

void Game::collideBricks()
{
    const Vector2 pos = ball_.position();
    const float r = cfg::BALL_RADIUS;

    // Resolve at most one brick per step. When the ball touches two bricks
    // at once (e.g. in the gap between them) only the closest one counts,
    // so a single hit can't break two bricks or bounce the ball twice.
    Brick* closest = nullptr;
    float closestDistSq = r * r;
    for (Brick& brick : bricks_) {
        if (!brick.alive()) {
            continue;
        }
        const float d = collision::distanceSq(pos, brick.rect);
        if (d < closestDistSq) {
            closestDistSq = d;
            closest = &brick;
        }
    }
    if (closest != nullptr) {
        hitBrick(*closest);
    }
}

void Game::hitBrick(Brick& brick)
{
    // Push the ball out along the axis it entered through, and make the
    // velocity on that axis point away from the brick.
    const Vector2 push = collision::separation(ball_.position(), cfg::BALL_RADIUS, brick.rect);
    Vector2 pos = ball_.position();
    Vector2 vel = ball_.velocity();
    pos.x += push.x;
    pos.y += push.y;
    if (push.x != 0.0f) {
        vel.x = std::copysign(vel.x, push.x);
    } else {
        vel.y = std::copysign(vel.y, push.y);
    }
    ball_.setPosition(pos);
    ball_.setDirection(vel);

    --brick.hp;
    if (!brick.alive()) {
        score_ += cfg::SCORE_PER_HP * brick.maxHp;
    }
}

void Game::drawBricks() const
{
    for (const Brick& brick : bricks_) {
        if (brick.alive()) {
            DrawRectangleRec(brick.rect, brickColor(brick));
        }
    }
}

void Game::drawHud() const
{
    DrawText(TextFormat("SCORE %d", score_), cfg::HUD_MARGIN, cfg::HUD_MARGIN, cfg::HUD_FONT_SIZE, cfg::COL_TEXT);
}
