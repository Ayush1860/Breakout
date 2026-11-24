#pragma once

#include "raylib.h"

#include <algorithm>
#include <cmath>

namespace collision {

// Squared distance from point p to the nearest point of rect r (0 if inside).
inline float distanceSq(Vector2 p, Rectangle r)
{
    const float nx = std::clamp(p.x, r.x, r.x + r.width);
    const float ny = std::clamp(p.y, r.y, r.y + r.height);
    const float dx = p.x - nx;
    const float dy = p.y - ny;
    return dx * dx + dy * dy;
}

inline bool circleRectOverlap(Vector2 center, float radius, Rectangle r)
{
    return distanceSq(center, r) < radius * radius;
}

// The smallest move that pushes the circle out of the rect, along one axis.
// Overlap is measured with the circle's bounding box: on each axis we take
// the cheaper way out (left/right, up/down), then pick the axis with the
// smaller overlap. That axis is the side the ball came in through, so it is
// also the velocity component to reflect.
inline Vector2 separation(Vector2 c, float radius, Rectangle r)
{
    const float pushLeft  = (c.x + radius) - r.x;
    const float pushRight = (r.x + r.width) - (c.x - radius);
    const float pushUp    = (c.y + radius) - r.y;
    const float pushDown  = (r.y + r.height) - (c.y - radius);

    const float dx = pushLeft < pushRight ? -pushLeft : pushRight;
    const float dy = pushUp < pushDown ? -pushUp : pushDown;

    if (std::abs(dx) < std::abs(dy)) {
        return {dx, 0.0f};
    }
    return {0.0f, dy};
}

} // namespace collision
