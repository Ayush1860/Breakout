#include "raylib.h"

#include "Constants.h"
#include "Game.h"

#include <algorithm>
#include <optional>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <utility>
#endif

namespace {

// The browser calls frame() as a plain C callback with no arguments, and
// emscripten_set_main_loop never returns to main, so the loop's state lives
// at file scope instead of on main's stack.
std::optional<Game> game;    // constructed after InitWindow
float accumulator = 0.0f;

#ifdef __EMSCRIPTEN__
// raylib's IsWindowFocused() is always true on the web: Emscripten's GLFW
// never forwards blur events. Hidden tabs also stop running frames, so we
// can't poll either. Instead, record the event and act on it next frame.
bool focusLost = false;

EM_BOOL onBlur(int, const EmscriptenFocusEvent*, void*)
{
    focusLost = true;
    return EM_FALSE;
}

EM_BOOL onVisibilityChange(int, const EmscriptenVisibilityChangeEvent* event, void*)
{
    if (event->hidden) {
        focusLost = true;
    }
    return EM_FALSE;
}

bool takeFocusLost()
{
    return std::exchange(focusLost, false);
}
#else
bool takeFocusLost()
{
    return !IsWindowFocused();
}
#endif

// Fixed timestep: physics always advances in FIXED_DT steps, so the game
// behaves the same at 30 or 144 FPS. Leftover time carries to the next frame.
void frame()
{
    accumulator += std::min(GetFrameTime(), cfg::MAX_FRAME_TIME);

    // Before any physics: the first frame back in a tab carries a long
    // frame time, which must not move a ball the player can't see.
    if (takeFocusLost()) {
        game->pause();
    }
    game->handleInput();
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
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, false, onBlur);
    emscripten_set_visibilitychange_callback(nullptr, false, onVisibilityChange);

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
