#pragma once

// High score persistence: localStorage in the browser, a text file on desktop.
namespace storage {

int loadHighScore();  // 0 if nothing is saved or it can't be read
void saveHighScore(int score);

} // namespace storage
