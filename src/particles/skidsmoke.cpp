#include "skidsmoke.h"
#include "room.h"

SkidSmoke::SkidSmoke(Room* room) : GameObject(room)
{
    sprite.load("sprites/smoke_small.png", "sprites/smoke_small.json");
    sprite.setOrigin(4, 4);
}

void SkidSmoke::step()
{
    sprite.frame += 0.2f;
    if (sprite.frame >= sprite.getFrameCount())
    {
        room->queueFree(this);
    }
    y -= 0.2f;
}

void SkidSmoke::draw(sf::RenderTarget& target)
{
    sprite.draw(target, x, y);
}