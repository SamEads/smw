#pragma once

#include "gameobject.h"

class Collision;

class QBlock : public GameObject
{
public:
    QBlock(Room* room, float x, float y);

    void step() override;
    void postStep() override;
    void draw(sf::RenderTarget& target, float interp) override;
    bool getWorldBounds(sf::FloatRect& bounds) const override;

private:
    bool wasHit = false;
    Collision* collision;
};
