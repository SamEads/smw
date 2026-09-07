#pragma once

#include "gameobject.h"

class QBlock : public GameObject
{
public:
    QBlock(Room* room, float x, float y);

    void step() override;
    void postStep() override;
    void draw(sf::RenderTarget& target) override;
    bool getWorldBounds(sf::FloatRect& bounds) const override;

private:
    bool wasHit = false;
};
