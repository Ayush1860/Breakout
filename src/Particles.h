#pragma once

#include "Constants.h"
#include "raylib.h"

#include <array>
#include <cstddef>
#include <random>

// Fixed-size particle pool. Spawning reuses slots in a ring, so there are
// no allocations during play; if the pool is ever full, the oldest particle
// is overwritten, which is invisible at this pool size.
class Particles {
public:
    // Bursts PARTICLES_PER_BRICK particles from random points inside area.
    void spawnBurst(Rectangle area, Color color, std::mt19937& rng);
    void update(float dt);
    void draw() const;
    void clear();

private:
    struct Particle {
        Vector2 pos{};
        Vector2 vel{};
        Color color{};
        float life = 0.0f;  // seconds left; 0 = free slot
    };

    std::array<Particle, cfg::PARTICLE_POOL_SIZE> pool_{};
    std::size_t next_ = 0;
};
