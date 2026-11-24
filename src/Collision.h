#pragma once

#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace collision {

// Nearest point of rect r to point p (p itself if it's inside).
inline Vector2 closestPoint(Vector2 p, Rectangle r)
{
    return {std::clamp(p.x, r.x, r.x + r.width), std::clamp(p.y, r.y, r.y + r.height)};
}

// Squared distance from point p to rect r (0 if inside).
inline float distanceSq(Vector2 p, Rectangle r)
{
    const Vector2 q = closestPoint(p, r);
    const float dx = p.x - q.x;
    const float dy = p.y - q.y;
    return dx * dx + dy * dy;
}

inline bool circleRectOverlap(Vector2 center, float radius, Rectangle r)
{
    return distanceSq(center, r) < radius * radius;
}

// True if a ball at c moving with vel is heading into r rather than away.
inline bool movingToward(Vector2 c, Vector2 vel, Rectangle r)
{
    const Vector2 q = closestPoint(c, r);
    return (q.x - c.x) * vel.x + (q.y - c.y) * vel.y >= 0.0f;
}

// The smallest move that pushes the circle out of the rect, along one axis.
// A ball can only have come in through a face it is moving toward (moving
// up-right: the bottom or the left face), so only those two are candidates.
// Of those, the one with less overlap (measured on the circle's bounding
// box) is the face it entered through, and the axis to reflect.
inline Vector2 separation(Vector2 c, float radius, Rectangle r, Vector2 vel)
{
    constexpr float NONE = std::numeric_limits<float>::infinity();

    float dx = NONE;
    if (vel.x > 0.0f) dx = r.x - (c.x + radius);                 // out through the left face
    if (vel.x < 0.0f) dx = (r.x + r.width) - (c.x - radius);     // out through the right face

    float dy = NONE;
    if (vel.y > 0.0f) dy = r.y - (c.y + radius);                 // out through the top face
    if (vel.y < 0.0f) dy = (r.y + r.height) - (c.y - radius);    // out through the bottom face

    if (std::abs(dx) < std::abs(dy)) {
        return {dx, 0.0f};
    }
    return {0.0f, dy};
}

} // namespace collision
