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
    sf::FloatRect collider;
    bool isOnSlopeSurface = false;
    float slopeAngle = 0.0f;

public:
    void move();
    bool getWorldBounds(sf::FloatRect& bounds) const override;
    bool isAtWall() const;
    bool isOnFloor() const;
    bool wasOnFloor() const;

protected:
    virtual void onCeilingHit() {}
    void setAtWall();

private:
    bool atWall = false;
    bool onFloor = false;
    bool previousFloor = false;
};