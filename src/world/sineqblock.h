#pragma once

#include "qblock.h"

// A QBlock that orbits its spawn point in a circle, purely to exercise
// Solid::resolveMovement()'s carry/push behavior on a moving solid
class SineQBlock : public QBlock
{
public:
    SineQBlock(Room* room, float x, float y);

    void postStep() override;

private:
    float originX, originY;
    float angle = 0.0f;
};
