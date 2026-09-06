#include "physicsentity.h"
#include "room.h"

PhysicsEntity::PhysicsEntity(Room *room) : GameObject(room)
{
}

void PhysicsEntity::move()
{
    isOnFloor = false;
    isAtWall = false;

    x += hspd;
    for (auto& c : room->collisions)
    {
        sf::FloatRect wall(
            { c.x, c.y },
            { c.width, c.height }
        );

        sf::FloatRect playerRect = collider;
        playerRect.position += { x, y };


        auto hit = wall.findIntersection(playerRect);

        if (hit.has_value())
        {
            if (hspd > 0.0f)
            {
                // moving right
                x -= hit->size.x;
            }
            else if (hspd < 0.0f)
            {
                // moving left
                x += hit->size.x;
            }

            hspd = 0.0f;
            isAtWall = true;
        }
    }

    y += vspd;

    for (auto& c : room->collisions)
    {
        bool oneWay = c.height <= 2.0f;
        float colHeight = oneWay ? 5.0f : c.height;
        sf::FloatRect wall(
            { c.x, c.y },
            { c.width, colHeight }
        );

        sf::FloatRect playerRect = collider;
        playerRect.position += { x, y };

        auto hit = wall.findIntersection(playerRect);

        if (hit.has_value())
        {
            if (oneWay)
            {
                if (vspd >= 0.0f)
                {
                    y -= hit->size.y;
                    vspd = 0.0f;
                    isOnFloor = true;
                }
                continue;
            }
            if (vspd > 0.0f)
            {
                // falling
                y -= hit->size.y;
                isOnFloor = true;
            }
            else if (vspd < 0.0f)
            {
                // moving upward
                y += hit->size.y;
            }

            vspd = 0.0f;
        }
    }
}