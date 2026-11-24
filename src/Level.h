#pragma once

#include "Brick.h"

#include <vector>

namespace level {

int count();

// Builds the bricks for level `index` (0-based), centred horizontally.
std::vector<Brick> load(int index);

} // namespace level
