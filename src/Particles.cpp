#include "Particles.h"

#include <cmath>

void Particles::spawnBurst(Rectangle area, Color color, std::mt19937& rng)
{
    std::uniform_real_distribution<float> px(area.x, area.x + area.width);
    std::uniform_real_distribution<float> py(area.y, area.y + area.height);
    std::uniform_real_distribution<float> angle(0.0f, 2.0f * PI);
    std::uniform_real_distribution<float> speed(cfg::PARTICLE_MIN_SPEED, cfg::PARTICLE_MAX_SPEED);

    for (int i = 0; i < cfg::PARTICLES_PER_BRICK; ++i) {
        const float a = angle(rng);
        const float s = speed(rng);
        pool_[next_] = {{px(rng), py(rng)}, {std::cos(a) * s, std::sin(a) * s}, color, cfg::PARTICLE_LIFE};
        next_ = (next_ + 1) % pool_.size();
    }
}

void Particles::update(float dt)
{
    for (Particle& p : pool_) {
        if (p.life <= 0.0f) {
            continue;
        }
        p.vel.y += cfg::PARTICLE_GRAVITY * dt;
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.life -= dt;
    }
}

void Particles::draw() const
{
    const float half = cfg::PARTICLE_SIZE / 2.0f;
    for (const Particle& p : pool_) {
        if (p.life <= 0.0f) {
            continue;
        }
        const float alpha = p.life / cfg::PARTICLE_LIFE;  // fades out linearly
        DrawRectangleV({p.pos.x - half, p.pos.y - half}, {cfg::PARTICLE_SIZE, cfg::PARTICLE_SIZE}, Fade(p.color, alpha));
    }
}

void Particles::clear()
{
    pool_.fill({});
    next_ = 0;
}
