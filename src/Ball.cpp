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
    recordTrail(dt);
    pos_.x += vel_.x * dt;
    pos_.y += vel_.y * dt;
}

void Ball::draw() const
{
    // Oldest trail circle is the faintest and smallest.
    for (int i = trailCount_ - 1; i >= 0; --i) {
        const float t = 1.0f - static_cast<float>(i + 1) / (cfg::TRAIL_LENGTH + 1);
        DrawCircleV(trail_[i], cfg::BALL_RADIUS * t, Fade(cfg::COL_BALL, cfg::TRAIL_ALPHA * t));
    }
    DrawCircleV(pos_, cfg::BALL_RADIUS, cfg::COL_BALL);
}

void Ball::reset()
{
    vel_ = {0.0f, 0.0f};
    launched_ = false;
    trailCount_ = 0;
    trailTimer_ = 0.0f;
}

void Ball::recordTrail(float dt)
{
    trailTimer_ += dt;
    if (trailTimer_ < cfg::TRAIL_SAMPLE_TIME) {
        return;
    }
    trailTimer_ -= cfg::TRAIL_SAMPLE_TIME;
    for (int i = cfg::TRAIL_LENGTH - 1; i > 0; --i) {
        trail_[i] = trail_[i - 1];
    }
    trail_[0] = pos_;
    if (trailCount_ < cfg::TRAIL_LENGTH) {
        ++trailCount_;
    }
}

void Ball::launch(float angle)
{
    launched_ = true;
    setDirection({std::sin(angle), -std::cos(angle)});
}

void Ball::setSpeed(float speed)
{
    speed_ = speed;
    if (launched_) {
        setDirection(vel_);
    }
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
