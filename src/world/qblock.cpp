#include "qblock.h"

#include <iostream>

#include "room.h"
#include "textures.h"
#include "mathhelper.h"

namespace
{
    constexpr float BLOCK_SIZE = 16.0f;
}

QBlock::QBlock(Room* room, float x, float y) : Solid(room)
{
    this->x = x;
    this->y = y;
    width = BLOCK_SIZE;
    height = BLOCK_SIZE;
}

void QBlock::step()
{
    if (bounceFramesRemaining > 0)
    {
        bounceFramesRemaining--;
        int elapsed = BOUNCE_TOTAL_FRAMES - bounceFramesRemaining;
        int half = BOUNCE_TOTAL_FRAMES / 2;
        bounceOffset = -2.0f * (elapsed <= half ? elapsed : BOUNCE_TOTAL_FRAMES - elapsed);
    }
    else
    {
        bounceOffset = 0.0f;
    }
}

void QBlock::draw(sf::RenderTarget& target, float interp)
{
    float xx = MathHelper::lerp(xPrevious, x, interp);
    float yy = MathHelper::lerp(yPrevious, y, interp);

    sf::Sprite sprite(Textures::get("sprites/qblock.png"));
    int frame = static_cast<int>(room->qblockAnimationFrame) % 4;
    sprite.setTextureRect({ { frame * 16, 0 }, { 16, 16 } });
    sprite.setPosition({ std::floorf(xx), std::floorf(yy + bounceOffset) });
    target.draw(sprite);
}

void QBlock::onHit(HitSide side, PhysicsEntity* entity)
{
    if (side != HitSide::Below || bounceFramesRemaining > 0)
        return;

    bounceFramesRemaining = BOUNCE_TOTAL_FRAMES;
    // TODO: spawn contents (coin/item/island) once those exist.
    std::cout << "QBlock hit from below\n";
}
