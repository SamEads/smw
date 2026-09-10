#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
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

    // Velocity imparted by a moving Solid carrying this entity this tick
    // (see Solid::resolveMovement). Zeroed at the top of every move() call,
    // so it only reads nonzero on a tick where this entity actually got
    // carried - used by the camera to tell "standing on solid ground" apart
    // from "standing on something that's moving".
    sf::Vector2f groundVelocity{ 0.0f, 0.0f };

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