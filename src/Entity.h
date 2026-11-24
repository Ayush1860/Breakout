#pragma once

// Common interface for things that move and draw themselves.
// Game stores entities by value (not through Entity pointers), so the
// base class only fixes the shape of the interface.
class Entity {
public:
    virtual ~Entity() = default;

    virtual void update(float dt) = 0;
    virtual void draw() const = 0;

protected:
    // Protected so an Entity can't be copied or moved on its own (no slicing),
    // while derived classes keep their default copy/move.
    Entity() = default;
    Entity(const Entity&) = default;
    Entity& operator=(const Entity&) = default;
    Entity(Entity&&) = default;
    Entity& operator=(Entity&&) = default;
};
