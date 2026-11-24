#include "Paddle.h"

#include "Constants.h"

#include <algorithm>

Paddle::Paddle()
{
    reset();
}

void Paddle::update(float dt)
{
    x_ += dir_ * cfg::PADDLE_SPEED * dt;
    x_ = std::clamp(x_, 0.0f, cfg::SCREEN_W - cfg::PADDLE_W);
}

void Paddle::draw() const
{
    DrawRectangleRec(rect(), cfg::COL_PADDLE);
}

void Paddle::reset()
{
    x_ = (cfg::SCREEN_W - cfg::PADDLE_W) / 2.0f;
    dir_ = 0.0f;
}

Rectangle Paddle::rect() const
{
    return {x_, cfg::PADDLE_Y, cfg::PADDLE_W, cfg::PADDLE_H};
}

float Paddle::centerX() const
{
    return x_ + cfg::PADDLE_W / 2.0f;
}
