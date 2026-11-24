#pragma once

#include "Entity.h"
#include "raylib.h"

class Ball : public Entity {
public:
    Ball();

    void update(float dt) override;
    void draw() const override;

    // Back to the "held on the paddle" state, not moving.
    void reset();

    // angle is measured from straight up, in radians (positive = right).
    void launch(float angle);
    bool isLaunched() const { return launched_; }

    Vector2 position() const { return pos_; }
    void setPosition(Vector2 pos) { pos_ = pos; }
    Vector2 velocity() const { return vel_; }

    // Keeps the current direction if the ball is moving.
    void setSpeed(float speed);

    // Points the ball along dir at its current speed. Every bounce goes
    // through here so the anti-stuck rule is applied in one place.
    void setDirection(Vector2 dir);

private:
    Vector2 pos_{};
    Vector2 vel_{};
    float speed_;
    bool launched_ = false;
};
