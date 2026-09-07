#pragma once

#include "../gameobject.h"
#include "../sprite.h"

class SkidSmoke : public GameObject
{
public:
    SkidSmoke(Room* room);

public:
    Sprite sprite;
    
public:
    void step() override;
    void draw(sf::RenderTarget& target) override;
};