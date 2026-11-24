#include "Level.h"

#include "Constants.h"

#include <array>
#include <string_view>

namespace level {
namespace {

// One string per row, one digit per column: the brick's starting HP.
// '0' means no brick.
using Layout = std::array<std::string_view, cfg::BRICK_ROWS>;

constexpr std::array<Layout, 1> LAYOUTS{{
    // Level 1: full grid, tougher bricks at the top
    {"3333333333",
     "3333333333",
     "2222222222",
     "2222222222",
     "1111111111",
     "1111111111"},
}};

// Typos in a layout are caught by the compiler, not at runtime.
constexpr bool isValid(const Layout& layout)
{
    for (std::string_view row : layout) {
        if (row.size() != static_cast<std::size_t>(cfg::BRICK_COLS)) {
            return false;
        }
        for (char c : row) {
            if (c < '0' || c > '0' + cfg::BRICK_MAX_HP) {
                return false;
            }
        }
    }
    return true;
}

constexpr bool allValid()
{
    for (const Layout& layout : LAYOUTS) {
        if (!isValid(layout)) {
            return false;
        }
    }
    return true;
}
static_assert(allValid(), "level layout has a wrong row length or HP digit");

} // namespace

int count()
{
    return static_cast<int>(LAYOUTS.size());
}

std::vector<Brick> load(int index)
{
    const Layout& layout = LAYOUTS.at(static_cast<std::size_t>(index));

    std::vector<Brick> bricks;
    bricks.reserve(cfg::BRICK_ROWS * cfg::BRICK_COLS);

    for (int row = 0; row < cfg::BRICK_ROWS; ++row) {
        for (int col = 0; col < cfg::BRICK_COLS; ++col) {
            const int hp = layout[row][col] - '0';
            if (hp == 0) {
                continue;
            }
            const Rectangle rect{
                cfg::BRICK_LEFT + col * (cfg::BRICK_W + cfg::BRICK_GAP),
                cfg::BRICK_TOP + row * (cfg::BRICK_H + cfg::BRICK_GAP),
                cfg::BRICK_W,
                cfg::BRICK_H,
            };
            bricks.push_back({rect, hp, hp});
        }
    }
    return bricks;
}

} // namespace level
