#include "physicsentity.h"
#include "room.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr float COLLISION_EPSILON = 0.01f;
constexpr float ONE_WAY_TOLERANCE = 5.0f;

bool edgeHeightAtX(const sf::Vector2f& first, const sf::Vector2f& second,
    float x, float& y)
{
    float dx = second.x - first.x;
    if (std::abs(dx) < COLLISION_EPSILON || x < std::min(first.x, second.x) ||
        x > std::max(first.x, second.x))
        return false;

    y = first.y + (second.y - first.y) * ((x - first.x) / dx);
    return true;
}

size_t edgeCount(const Collision& collision)
{
    return collision.shape == CollisionShape::Polygon ? collision.points.size() :
        collision.points.size() - 1;
}

bool solidVerticalEdge(const sf::Vector2f& first, const sf::Vector2f& second,
    float top, float bottom)
{
    return bottom > std::min(first.y, second.y) && top < std::max(first.y, second.y);
}
}

PhysicsEntity::PhysicsEntity(Room *room) : GameObject(room)
{
}

void PhysicsEntity::move()
{
    float previousX = x;
    float previousY = y;
    wasOnFloor = isOnFloor;
    isOnFloor = false;
    isAtWall = false;
    isOnSlopeSurface = false;
    slopeAngle = 0.0f;

    x += hspd;
    for (auto& c : room->collisions)
    {
        if (c.shape != CollisionShape::Rectangle)
        {
            if (c.shape == CollisionShape::Polygon)
            {
                float previousLeft = previousX + collider.position.x;
                float previousRight = previousLeft + collider.size.x;
                float currentLeft = x + collider.position.x;
                float currentRight = currentLeft + collider.size.x;
                float top = y + collider.position.y;
                float bottom = top + collider.size.y;

                for (size_t index = 0; index < edgeCount(c); ++index)
                {
                    const auto& first = c.points[index];
                    const auto& second = c.points[(index + 1) % c.points.size()];
                    if (std::abs(second.x - first.x) >= COLLISION_EPSILON ||
                        !solidVerticalEdge(first, second, top, bottom))
                        continue;

                    if (hspd > 0.0f && previousRight <= first.x && currentRight > first.x)
                    {
                        x = first.x - collider.position.x - collider.size.x;
                        hspd = 0.0f;
                        isAtWall = true;
                    }
                    else if (hspd < 0.0f && previousLeft >= first.x && currentLeft < first.x)
                    {
                        x = first.x - collider.position.x;
                        hspd = 0.0f;
                        isAtWall = true;
                    }
                }
            }
            continue;
        }

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
        if (c.shape != CollisionShape::Rectangle)
        {
            if (c.points.size() >= 2)
            {
                float centerX = x + collider.position.x + collider.size.x * 0.5f;
                float previousCenterX = previousX + collider.position.x + collider.size.x * 0.5f;
                float previousBottom = previousY + collider.position.y + collider.size.y;
                float currentBottom = y + collider.position.y + collider.size.y;
                float previousTop = previousY + collider.position.y;
                float currentTop = y + collider.position.y;
                float currentFloor = 0.0f;
                float previousFloor = 0.0f;
                bool hasCurrentFloor = false;
                bool hasPreviousFloor = false;
                float currentCeiling = 0.0f;
                float previousCeiling = 0.0f;
                bool hasCurrentCeiling = false;
                bool hasPreviousCeiling = false;

                for (size_t index = 0; index < edgeCount(c); ++index)
                {
                    const auto& first = c.points[index];
                    const auto& second = c.points[(index + 1) % c.points.size()];
                    float candidate = 0.0f;
                    if (edgeHeightAtX(first, second, centerX, candidate))
                    {
                        if (!hasCurrentFloor || std::abs(candidate - previousBottom) <
                            std::abs(currentFloor - previousBottom))
                        {
                            currentFloor = candidate;
                            hasCurrentFloor = true;
                        }
                        if (!hasCurrentCeiling || std::abs(candidate - previousTop) <
                            std::abs(currentCeiling - previousTop))
                        {
                            currentCeiling = candidate;
                            hasCurrentCeiling = true;
                        }
                    }
                    if (edgeHeightAtX(first, second, previousCenterX, candidate))
                    {
                        if (!hasPreviousFloor || std::abs(candidate - previousBottom) <
                            std::abs(previousFloor - previousBottom))
                        {
                            previousFloor = candidate;
                            hasPreviousFloor = true;
                        }
                        if (!hasPreviousCeiling || std::abs(candidate - previousTop) <
                            std::abs(previousCeiling - previousTop))
                        {
                            previousCeiling = candidate;
                            hasPreviousCeiling = true;
                        }
                    }
                }

                bool wasOnPolyline = c.shape == CollisionShape::Polyline && hasPreviousFloor &&
                    std::abs(previousBottom - previousFloor) <= 1.0f;
                bool canCatchDownhill = wasOnPolyline && hasCurrentFloor &&
                    currentFloor >= previousFloor &&
                    currentFloor - previousFloor <= ONE_WAY_TOLERANCE;
                if (vspd >= 0.0f && hasCurrentFloor &&
                    (c.shape == CollisionShape::Polyline || hasPreviousFloor) &&
                    previousBottom <= (hasPreviousFloor ? previousFloor : currentFloor) +
                        (c.shape == CollisionShape::Polyline ? ONE_WAY_TOLERANCE : 0.0f) &&
                    (currentBottom >= currentFloor || canCatchDownhill))
                {
                    y = currentFloor - collider.position.y - collider.size.y;
                    vspd = 0.0f;
                    isOnFloor = true;
                    for (size_t index = 0; index < edgeCount(c); ++index)
                    {
                        const auto& first = c.points[index];
                        const auto& second = c.points[(index + 1) % c.points.size()];
                        float edgeY = 0.0f;
                        if (edgeHeightAtX(first, second, centerX, edgeY) &&
                            std::abs(edgeY - currentFloor) < COLLISION_EPSILON)
                        {
                            slopeAngle = std::atan2(second.y - first.y, second.x - first.x);
                            if (slopeAngle > 3.14159265f * 0.5f)
                                slopeAngle -= 3.14159265f;
                            else if (slopeAngle < -3.14159265f * 0.5f)
                                slopeAngle += 3.14159265f;
                            isOnSlopeSurface = std::abs(slopeAngle) > COLLISION_EPSILON;
                            break;
                        }
                    }
                }
                else if (c.shape == CollisionShape::Polygon && vspd < 0.0f &&
                    hasCurrentCeiling && hasPreviousCeiling &&
                    previousTop >= previousCeiling && currentTop <= currentCeiling)
                {
                    y = currentCeiling - collider.position.y;
                    vspd = 0.0f;
                    onCeilingHit();
                }
            }
            continue;
        }

        bool oneWay = c.height <= 2.0f;
        float colHeight = oneWay ? 5.0f : c.height;
        sf::FloatRect wall(
            { c.x, c.y },
            { c.width, colHeight }
        );

        sf::FloatRect playerRect = collider;
        if (oneWay)
        {
            playerRect.position.y += playerRect.size.y - 5;
            playerRect.size.y = 5;
        }
        playerRect.position += { x, y };

        auto hit = wall.findIntersection(playerRect);

        if (hit.has_value())
        {
            if (oneWay)
            {
                if (vspd >= 0.0f)
                {
                    y -= hit->size.y;
                    isOnFloor = true;
                }
                if (vspd >= 0.0f)
                    vspd = 0.0f;
                continue;
            }
            if (vspd > 0.0f)
            {
                // falling
                y -= hit->size.y;
                isOnFloor = true;
            }
            // Solid rectangle tops are one-way from above: jumping upward
            // does not get stopped by the horizontal surface.
            if (vspd > 0.0f)
                vspd = 0.0f;
        }
    }

    if (!isOnFloor && wasOnFloor && vspd >= 0.0f)
    {
        float centerX = x + collider.position.x + collider.size.x * 0.5f;
        float feetY = y + collider.position.y + collider.size.y;
        float supportY = 0.0f;
        float supportDistance = 0.0f;
        bool foundSupport = false;

        for (auto& c : room->collisions)
        {
            if (c.shape == CollisionShape::Rectangle)
            {
                if (c.width <= 0.0f || centerX < c.x || centerX > c.x + c.width)
                    continue;

                float distance = std::abs(feetY - c.y);
                if (!foundSupport || distance < supportDistance)
                {
                    foundSupport = true;
                    supportDistance = distance;
                    supportY = c.y;
                }
                continue;
            }

            for (size_t index = 0; index < edgeCount(c); ++index)
            {
                float candidateY = 0.0f;
                const auto& first = c.points[index];
                const auto& second = c.points[(index + 1) % c.points.size()];
                if (!edgeHeightAtX(first, second, centerX, candidateY))
                    continue;

                float distance = std::abs(feetY - candidateY);
                if (!foundSupport || distance < supportDistance)
                {
                    foundSupport = true;
                    supportDistance = distance;
                    supportY = candidateY;
                }
            }
        }

        if (foundSupport && supportDistance <= ONE_WAY_TOLERANCE)
        {
            y = supportY - collider.position.y - collider.size.y;
            vspd = 0.0f;
            isOnFloor = true;
        }
    }

    if (isOnFloor)
        airborneFrames = 0;
    else
        airborneFrames++;
}