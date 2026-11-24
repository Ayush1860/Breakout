#include "raylib.h"

#include "Constants.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace {

// One iteration of the main loop. Kept as a free function so the browser
// can drive it via emscripten_set_main_loop.
void frame()
{
    BeginDrawing();
    ClearBackground(cfg::COL_BACKGROUND);
    EndDrawing();
}

} // namespace

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(cfg::SCREEN_W, cfg::SCREEN_H, cfg::WINDOW_TITLE);

#ifdef __EMSCRIPTEN__
    // The browser owns the loop: frame() runs once per requestAnimationFrame.
    // simulate_infinite_loop = 1 means this call never returns.
    emscripten_set_main_loop(frame, 0, 1);
#else
    while (!WindowShouldClose()) {
        frame();
    }
#endif

    CloseWindow();
    return 0;
}
