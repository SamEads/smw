#include "sineqblock.h"

#include <cmath>

namespace
{
    constexpr float ORBIT_RADIUS = 32.0f;
    constexpr float ANGULAR_SPEED = 0.02f; // radians per frame
}

SineQBlock::SineQBlock(Room* room, float x, float y) : QBlock(room, x, y),
    originX(x), originY(y)
{
}

void SineQBlock::postStep()
{
    angle += ANGULAR_SPEED;

    x = originX + std::sin(angle) * ORBIT_RADIUS;
    y = originY + std::cos(angle) * ORBIT_RADIUS - ORBIT_RADIUS;

    resolveMovement();

    QBlock::step();
}
