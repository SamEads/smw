#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "collisioncomponent.h"
#include "enums.h"

class Room;

class GameObject
{
public:
    GameObject() = default;
    GameObject(Room* room);

public:
    Room* room;
    float x, y;
    float xPrevious, yPrevious;
    int depth = 0;

public:
    virtual void step() {}
    virtual void postStep() {}
    virtual void draw(sf::RenderTarget& target, float interp) {}
    virtual bool getWorldBounds(sf::FloatRect& bounds) const { return false; }
    virtual ObjectCategory getCategory() const { return ObjectCategory::WORLD; }
};