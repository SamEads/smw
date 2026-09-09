#include "qblock.h"

#include <iostream>

#include "player.h"
#include "room.h"
#include "textures.h"
#include "collision.h"

constexpr float BLOCK_SIZE = 16.0f;

QBlock::QBlock(Room* room, float x, float y) : GameObject(room)
{
    this->x = x;
    this->y = y;

    collision = room->create<Collision>();
    auto functor = []()
    {
        return false;
    };
    collision->x = x;
    collision->y = y;
    collision->width = 16;
    collision->height = 16;
}

void QBlock::step()
{
}

void QBlock::draw(sf::RenderTarget& target, float interp)
{
    sf::Sprite sprite(Textures::get("sprites/qblock.png"));
    int frame = static_cast<int>(room->qblockAnimationFrame) % 4;
    sprite.setTextureRect({ { frame * 16, 0 }, { 16, 16 } });
    sprite.setPosition({ x, y });
    target.draw(sprite);
}

bool QBlock::getWorldBounds(sf::FloatRect& bounds) const
{
    bounds = sf::FloatRect({ x, y }, { BLOCK_SIZE, BLOCK_SIZE });
    return true;
}