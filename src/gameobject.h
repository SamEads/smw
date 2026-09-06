#pragma once

#include <SFML/Graphics/RenderTarget.hpp>

class Room;

class GameObject
{
public:
    GameObject(Room* room);

public:
    Room* room;
    int depth = 0;

public:
    virtual void draw(sf::RenderTarget& target) {}
};