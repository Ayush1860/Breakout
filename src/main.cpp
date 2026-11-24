#include "raylib.h"

#include "Constants.h"
#include "Game.h"

#include <algorithm>
#include <optional>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace {

// The browser calls frame() as a plain C callback with no arguments, and
// emscripten_set_main_loop never returns to main, so the loop's state lives
// at file scope instead of on main's stack.
std::optional<Game> game;    // constructed after InitWindow
float accumulator = 0.0f;

// Fixed timestep: physics always advances in FIXED_DT steps, so the game
// behaves the same at 30 or 144 FPS. Leftover time carries to the next frame.
void frame()
{
    accumulator += std::min(GetFrameTime(), cfg::MAX_FRAME_TIME);

    game->readInput();
    while (accumulator >= cfg::FIXED_DT) {
        game->update(cfg::FIXED_DT);
        accumulator -= cfg::FIXED_DT;
    }

    BeginDrawing();
    ClearBackground(cfg::COL_BACKGROUND);
    game->draw();
    EndDrawing();
}

} // namespace

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(cfg::SCREEN_W, cfg::SCREEN_H, cfg::WINDOW_TITLE);
    game.emplace();

#ifdef __EMSCRIPTEN__
    // The browser owns the loop: frame() runs once per requestAnimationFrame.
    // simulate_infinite_loop = 1 means this call never returns.
    emscripten_set_main_loop(frame, 0, 1);
#else
    while (!WindowShouldClose()) {
        frame();
    }
#endif

    game.reset();
    CloseWindow();
    return 0;
}
