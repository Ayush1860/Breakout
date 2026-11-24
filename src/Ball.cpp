#include "Ball.h"

#include "Constants.h"

#include <cmath>

Ball::Ball()
    : speed_(cfg::BALL_START_SPEED)
{
}

void Ball::update(float dt)
{
    if (!launched_) {
        return;
    }
    pos_.x += vel_.x * dt;
    pos_.y += vel_.y * dt;
}

void Ball::draw() const
{
    DrawCircleV(pos_, cfg::BALL_RADIUS, cfg::COL_BALL);
}

void Ball::reset()
{
    vel_ = {0.0f, 0.0f};
    launched_ = false;
}

void Ball::launch(float angle)
{
    launched_ = true;
    setDirection({std::sin(angle), -std::cos(angle)});
}

void Ball::setDirection(Vector2 dir)
{
    const float len = std::hypot(dir.x, dir.y);
    if (len == 0.0f) {
        return;
    }
    float nx = dir.x / len;
    float ny = dir.y / len;

    // A nearly horizontal ball takes ages to come back down and can bounce
    // between the side walls forever, so keep a minimum vertical component.
    if (std::abs(ny) < cfg::MIN_VERTICAL_RATIO) {
        ny = std::copysign(cfg::MIN_VERTICAL_RATIO, ny);
        nx = std::copysign(std::sqrt(1.0f - ny * ny), nx);
    }

    vel_ = {nx * speed_, ny * speed_};
}
