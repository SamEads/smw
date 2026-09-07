#pragma once

#include <SFML/Graphics/Rect.hpp>
#include "gameobject.h"

class PhysicsEntity : public GameObject
{
public:
    PhysicsEntity(Room* room);

public:
    float vspd = 0.0f, hspd = 0.0f;
    float grav = 0.0f;
    bool isAtWall = false;
    bool isOnFloor = false;
    bool wasOnFloor = false;
    int airborneFrames = 0;
    sf::FloatRect collider;
    bool isOnSlopeSurface = false;
    float slopeAngle = 0.0f;

public:
    void move();

protected:
    virtual void onCeilingHit() {}
};