#include "Storage.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#include <fstream>
#endif

namespace storage {

#ifdef __EMSCRIPTEN__

// localStorage can throw (e.g. storage disabled by the browser), so both
// calls fail quietly: the game still works, it just won't remember.
int loadHighScore()
{
    return EM_ASM_INT({
        try {
            return parseInt(localStorage.getItem('breakout.highScore'), 10) || 0;
        } catch (e) {
            return 0;
        }
    });
}

void saveHighScore(int score)
{
    EM_ASM({
        try {
            localStorage.setItem('breakout.highScore', String($0));
        } catch (e) {
        }
    }, score);
}

#else

namespace {
const char* const FILE_NAME = "highscore.txt";  // next to where the game is run
}

int loadHighScore()
{
    std::ifstream in(FILE_NAME);
    int score = 0;
    if (in >> score && score > 0) {
        return score;
    }
    return 0;
}

void saveHighScore(int score)
{
    std::ofstream out(FILE_NAME);
    out << score << '\n';
}

#endif

} // namespace storage
