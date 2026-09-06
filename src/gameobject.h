#pragma once

#include <SFML/Graphics/RenderTarget.hpp>

class Room;

class GameObject
{
public:
    GameObject(Room* room);

public:
    Room* room;
    float x, y;
    float xPrevious, yPrevious;
    int depth = 0;

public:
    virtual void step() {}
    virtual void draw(sf::RenderTarget& target) {}
};