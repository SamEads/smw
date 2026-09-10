#pragma once

#include "solid.h"

class QBlock : public Solid
{
public:
    QBlock(Room* room, float x, float y);

    void step() override;
    void draw(sf::RenderTarget& target, float interp) override;
    void onHit(HitSide side, PhysicsEntity* entity) override;
    ObjectCategory getCategory() const override { return ObjectCategory::GIZMO; }

protected:
    static constexpr int BOUNCE_TOTAL_FRAMES = 8;

    int bounceFramesRemaining = 0;
    float bounceOffset = 0.0f;
};
