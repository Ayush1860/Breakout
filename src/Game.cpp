#include "Game.h"

#include "Collision.h"
#include "Constants.h"
#include "Level.h"
#include "Storage.h"

#include <algorithm>
#include <cmath>

namespace {

float ballSpeedForLevel(int level)
{
    const float speed = cfg::BALL_START_SPEED * std::pow(cfg::BALL_SPEED_LEVEL_MULT, static_cast<float>(level));
    return std::min(speed, cfg::BALL_MAX_SPEED);
}

void drawTextCentered(const char* text, int y, int fontSize, Color color)
{
    const int width = MeasureText(text, fontSize);
    DrawText(text, (cfg::SCREEN_W - width) / 2, y, fontSize, color);
}

// Draws "LABEL value" at x and returns the x just past the value.
int drawLabelValue(const char* label, int value, int x, int y, Color valueColor)
{
    DrawText(label, x, y, cfg::HUD_FONT_SIZE, cfg::COL_TEXT_MUTED);
    x += MeasureText(label, cfg::HUD_FONT_SIZE) + cfg::HUD_LABEL_GAP;
    const char* text = TextFormat("%d", value);
    DrawText(text, x, y, cfg::HUD_FONT_SIZE, valueColor);
    return x + MeasureText(text, cfg::HUD_FONT_SIZE);
}

} // namespace

Game::Game()
    : lives_(cfg::START_LIVES)
    , bestScore_(storage::loadHighScore())
    , rng_(std::random_device{}())
{
    startLevel(0);  // the menu shows level 1 behind the title
}

// ---------------------------------------------------------------- input ----

void Game::handleInput()
{
    paddleDir_ = 0.0f;
    if (IsKeyDown(KEY_LEFT))  paddleDir_ -= 1.0f;
    if (IsKeyDown(KEY_RIGHT)) paddleDir_ += 1.0f;

    // Key presses are one-frame events, but a frame can run zero or several
    // physics steps. Handling them here, once per frame, means a press is
    // never missed and never applied twice.
    switch (state_) {
    case GameState::Menu:
        if (IsKeyPressed(KEY_SPACE)) newGame();
        break;
    case GameState::Playing:
        if (IsKeyPressed(KEY_P)) {
            pause();
        } else if (IsKeyPressed(KEY_SPACE) && !ball_.isLaunched()) {
            launchBall();
        }
        break;
    case GameState::Paused:
        if (IsKeyPressed(KEY_P)) state_ = GameState::Playing;
        break;
    case GameState::LevelClear:
        if (IsKeyPressed(KEY_SPACE)) {
            startLevel(level_ + 1);
            state_ = GameState::Playing;
        }
        break;
    case GameState::GameOver:
    case GameState::Win:
        if (IsKeyPressed(KEY_R)) newGame();
        break;
    }
}

// ----------------------------------------------------------------- flow ----

void Game::pause()
{
    if (state_ == GameState::Playing) {
        state_ = GameState::Paused;
    }
}

void Game::update(float dt)
{
    switch (state_) {
    case GameState::Playing:
        updatePlaying(dt);
        updateEffects(dt);
        break;
    case GameState::LevelClear:
    case GameState::GameOver:
    case GameState::Win:
        updateEffects(dt);  // let the last burst and shake settle
        break;
    case GameState::Menu:
    case GameState::Paused:
        break;  // everything frozen
    }
}

void Game::newGame()
{
    score_ = 0;
    lives_ = cfg::START_LIVES;
    startLevel(0);
    state_ = GameState::Playing;
}

void Game::startLevel(int index)
{
    level_ = index;
    bricks_ = level::load(index);
    particles_.clear();
    shake_ = 0.0f;
    shakeOffset_ = {0.0f, 0.0f};
    paddle_.reset();
    ball_.reset();
    ball_.setSpeed(ballSpeedForLevel(index));
    holdBallOnPaddle();
}

void Game::updatePlaying(float dt)
{
    paddle_.setDirection(paddleDir_);
    paddle_.update(dt);

    if (!ball_.isLaunched()) {
        holdBallOnPaddle();
        return;
    }

    ball_.update(dt);
    collideWalls();
    collideBricks();
    collidePaddle();

    if (ball_.position().y - cfg::BALL_RADIUS > cfg::SCREEN_H) {
        loseLife();
    } else if (allBricksCleared()) {
        finishLevel();
    }
}

void Game::loseLife()
{
    --lives_;
    addShake(cfg::SHAKE_LIFE_LOST);
    if (lives_ <= 0) {
        state_ = GameState::GameOver;
        return;
    }
    ball_.reset();
    holdBallOnPaddle();
}

void Game::finishLevel()
{
    const bool lastLevel = level_ + 1 >= level::count();
    state_ = lastLevel ? GameState::Win : GameState::LevelClear;
}

// ------------------------------------------------------ ball & collision ----

void Game::holdBallOnPaddle()
{
    ball_.setPosition({paddle_.centerX(), cfg::PADDLE_Y - cfg::BALL_RADIUS});
}

void Game::launchBall()
{
    std::uniform_real_distribution<float> spread(-cfg::LAUNCH_MAX_ANGLE, cfg::LAUNCH_MAX_ANGLE);
    ball_.launch(spread(rng_));
}

void Game::collideWalls()
{
    Vector2 pos = ball_.position();
    Vector2 vel = ball_.velocity();
    const float r = cfg::BALL_RADIUS;
    bool bounced = false;

    // Push the ball back inside and force the velocity's sign (instead of
    // negating it), so a ball still touching the wall can't flip back out.
    if (pos.x - r < 0.0f) {
        pos.x = r;
        vel.x = std::abs(vel.x);
        bounced = true;
    } else if (pos.x + r > cfg::SCREEN_W) {
        pos.x = cfg::SCREEN_W - r;
        vel.x = -std::abs(vel.x);
        bounced = true;
    }
    if (pos.y - r < cfg::PLAY_TOP) {
        pos.y = cfg::PLAY_TOP + r;
        vel.y = std::abs(vel.y);
        bounced = true;
    }

    if (bounced) {
        ball_.setPosition(pos);
        ball_.setDirection(vel);
    }
}

void Game::collidePaddle()
{
    if (ball_.velocity().y <= 0.0f) {
        return;  // only bounce a ball that is coming down
    }
    const Vector2 pos = ball_.position();
    if (!collision::circleRectOverlap(pos, cfg::BALL_RADIUS, paddle_.rect())) {
        return;
    }

    // Hit position from -1 (left edge) to +1 (right edge) picks the angle:
    // centre sends the ball straight up, the edges at MAX_BOUNCE_ANGLE.
    const float halfW = cfg::PADDLE_W / 2.0f;
    const float offset = std::clamp((pos.x - paddle_.centerX()) / halfW, -1.0f, 1.0f);
    const float angle = offset * cfg::MAX_BOUNCE_ANGLE;

    ball_.setPosition({pos.x, cfg::PADDLE_Y - cfg::BALL_RADIUS});
    ball_.setDirection({std::sin(angle), -std::cos(angle)});
}

void Game::collideBricks()
{
    const Vector2 pos = ball_.position();
    const Vector2 vel = ball_.velocity();
    const float r = cfg::BALL_RADIUS;

    // Resolve at most one brick per step. When the ball touches two bricks
    // at once (e.g. in the gap between them) only the closest one counts,
    // so a single hit can't break two bricks or bounce the ball twice.
    // Bricks the ball is already leaving are skipped: it can still overlap
    // one it was bounced off last step, and that one isn't a new hit.
    Brick* closest = nullptr;
    float closestDistSq = r * r;
    for (Brick& brick : bricks_) {
        if (!brick.alive() || !collision::movingToward(pos, vel, brick.rect)) {
            continue;
        }
        const float d = collision::distanceSq(pos, brick.rect);
        if (d < closestDistSq) {
            closestDistSq = d;
            closest = &brick;
        }
    }
    if (closest != nullptr) {
        hitBrick(*closest);
    }
}

void Game::hitBrick(Brick& brick)
{
    // Push the ball out through the face it entered by, and make the
    // velocity on that axis point away from the brick.
    Vector2 pos = ball_.position();
    Vector2 vel = ball_.velocity();
    const Vector2 push = collision::separation(pos, cfg::BALL_RADIUS, brick.rect, vel);
    pos.x += push.x;
    pos.y += push.y;
    if (push.x != 0.0f) {
        vel.x = std::copysign(vel.x, push.x);
    } else {
        vel.y = std::copysign(vel.y, push.y);
    }
    ball_.setPosition(pos);
    ball_.setDirection(vel);

    --brick.hp;
    brick.flash = cfg::HIT_FLASH_TIME;
    if (brick.alive()) {
        return;
    }

    score_ += cfg::SCORE_PER_HP * brick.maxHp;
    particles_.spawnBurst(brick.rect, brickColor(brick.maxHp, false), rng_);
    addShake(cfg::SHAKE_BRICK_BREAK);

    // Saved as soon as it's beaten (a tiny write), so reloading or closing
    // the page mid-game never loses it.
    if (score_ > bestScore_) {
        bestScore_ = score_;
        storage::saveHighScore(bestScore_);
    }
}

bool Game::allBricksCleared() const
{
    return std::none_of(bricks_.begin(), bricks_.end(), [](const Brick& b) { return b.alive(); });
}

// -------------------------------------------------------------- effects ----

void Game::updateEffects(float dt)
{
    particles_.update(dt);
    for (Brick& brick : bricks_) {
        brick.flash = std::max(0.0f, brick.flash - dt);
    }

    // Shake: jitter the camera in a random direction, shrinking each step.
    // Decaying per fixed step (not per rendered frame) keeps it FPS-independent.
    shake_ *= cfg::SHAKE_DECAY;
    if (shake_ < cfg::SHAKE_CUTOFF) {
        shake_ = 0.0f;
    }
    std::uniform_real_distribution<float> angle(0.0f, 2.0f * PI);
    const float a = angle(rng_);
    shakeOffset_ = {std::cos(a) * shake_, std::sin(a) * shake_};
}

void Game::addShake(float amount)
{
    shake_ = std::max(shake_, amount);
}

// -------------------------------------------------------------- drawing ----

void Game::draw() const
{
    drawHud();
    drawWorld();

    switch (state_) {
    case GameState::Menu:
        drawOverlay("BREAKOUT", cfg::COL_TEXT, "Press SPACE to start");
        break;
    case GameState::Playing:
        break;
    case GameState::Paused:
        drawOverlay("PAUSED", cfg::COL_TEXT, "Press P to resume");
        break;
    case GameState::LevelClear:
        drawOverlay(TextFormat("LEVEL %d CLEAR", level_ + 1), cfg::COL_SCORE,
                    TextFormat("Press SPACE for level %d", level_ + 2));
        break;
    case GameState::GameOver:
        drawOverlay("GAME OVER", cfg::COL_ALERT, TextFormat("Score %d  -  Press R to restart", score_));
        break;
    case GameState::Win:
        drawOverlay("YOU WIN", cfg::COL_SCORE, TextFormat("Final score %d  -  Press R to play again", score_));
        break;
    }

    drawControlsHint();  // on top of any overlay
}

// The play field shakes; the HUD, overlays and hint stay put.
void Game::drawWorld() const
{
    Camera2D camera{};
    camera.offset = shakeOffset_;
    camera.zoom = 1.0f;

    BeginMode2D(camera);
    drawBricks();
    particles_.draw();
    paddle_.draw();
    ball_.draw();
    EndMode2D();
}

void Game::drawBricks() const
{
    for (const Brick& brick : bricks_) {
        if (brick.alive()) {
            DrawRectangleRec(brick.rect, brickColor(brick));
        }
    }
}

void Game::drawHud() const
{
    const int y = static_cast<int>((cfg::HUD_HEIGHT - cfg::HUD_FONT_SIZE) / 2.0f);

    // Left: score and best
    int x = drawLabelValue("SCORE", score_, cfg::HUD_MARGIN, y, cfg::COL_SCORE);
    drawLabelValue("BEST", bestScore_, x + cfg::HUD_ITEM_GAP, y, cfg::COL_TEXT);

    // Centre: level
    drawTextCentered(TextFormat("LEVEL %d / %d", level_ + 1, level::count()), y, cfg::HUD_FONT_SIZE, cfg::COL_TEXT);

    // Right: one dot per remaining life
    const float dotY = cfg::HUD_HEIGHT / 2.0f;
    for (int i = 0; i < lives_; ++i) {
        const float dotX = cfg::SCREEN_W - cfg::HUD_MARGIN - cfg::LIFE_DOT_RADIUS - i * cfg::LIFE_DOT_SPACING;
        DrawCircleV({dotX, dotY}, cfg::LIFE_DOT_RADIUS, cfg::COL_ALERT);
    }

    DrawLineV({0.0f, cfg::HUD_HEIGHT}, {static_cast<float>(cfg::SCREEN_W), cfg::HUD_HEIGHT}, cfg::COL_HUD_DIVIDER);
}

void Game::drawControlsHint() const
{
    // raylib's built-in font is ASCII-only, so arrows are spelled out.
    drawTextCentered("LEFT / RIGHT  move      SPACE  launch      P  pause",
                     cfg::SCREEN_H - cfg::HINT_FONT_SIZE - cfg::HINT_MARGIN, cfg::HINT_FONT_SIZE, cfg::COL_TEXT_MUTED);
}

void Game::drawOverlay(const char* title, Color titleColor, const char* subtitle) const
{
    const float playH = cfg::SCREEN_H - cfg::PLAY_TOP;
    DrawRectangleRec({0.0f, cfg::PLAY_TOP, static_cast<float>(cfg::SCREEN_W), playH}, cfg::COL_OVERLAY);

    const int blockH = cfg::TITLE_FONT_SIZE + cfg::TITLE_GAP + cfg::SUBTITLE_FONT_SIZE;
    const int top = static_cast<int>(cfg::PLAY_TOP + (playH - blockH) / 2.0f);
    drawTextCentered(title, top, cfg::TITLE_FONT_SIZE, titleColor);
    drawTextCentered(subtitle, top + cfg::TITLE_FONT_SIZE + cfg::TITLE_GAP, cfg::SUBTITLE_FONT_SIZE, cfg::COL_TEXT_MUTED);
}
