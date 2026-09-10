#include "physicsentity.h"
#include "room.h"
#include "solid.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float COLLISION_EPSILON = 0.01f;
    constexpr float ONE_WAY_TOLERANCE = 5.0f;

    bool edgeHeightAtX(const sf::Vector2f& first, const sf::Vector2f& second, float x, float& y)
    {
        float dx = second.x - first.x;
        if (std::abs(dx) < COLLISION_EPSILON || x < std::min(first.x, second.x) ||
            x > std::max(first.x, second.x))
            return false;

        y = first.y + (second.y - first.y) * ((x - first.x) / dx);
        return true;
    }

    size_t edgeCount(const Solid& collision)
    {
        return collision.shape == CollisionShape::Polygon ? collision.points.size() :
            collision.points.size() - 1;
    }

    bool solidVerticalEdge(const sf::Vector2f& first, const sf::Vector2f& second, float top, float bottom)
    {
        return bottom > std::min(first.y, second.y) && top < std::max(first.y, second.y);
    }
}

PhysicsEntity::PhysicsEntity(Room *room) : GameObject(room)
{
}

bool PhysicsEntity::getWorldBounds(sf::FloatRect& bounds) const
{
    bounds = collider;
    bounds.position += { x, y };
    return true;
}

bool PhysicsEntity::isAtWall() const { return atWall; }

bool PhysicsEntity::isOnFloor() const { return onFloor; }

bool PhysicsEntity::wasOnFloor() const { return previousFloor; }

void PhysicsEntity::setAtWall() { atWall = true; }

void PhysicsEntity::move()
{
    float previousX = x;
    float previousY = y;
    slopeAngle = 0.0f;
    previousFloor = onFloor;
    onFloor = false;
    atWall = false;
    isOnSlopeSurface = false;
    groundVelocity = { 0.0f, 0.0f };

    // X COLLISIONS
    x += hspd;
    for (auto& o : room->objects)
    {
        Solid* solidPtr = dynamic_cast<Solid*>(o.get());
        if (!solidPtr || !solidPtr->collidable) continue;
        Solid& c = *solidPtr;
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
                        setAtWall();
                        c.onHit(HitSide::Left, this);
                    }
                    else if (hspd < 0.0f && previousLeft >= first.x && currentLeft < first.x)
                    {
                        x = first.x - collider.position.x;
                        hspd = 0.0f;
                        setAtWall();
                        c.onHit(HitSide::Right, this);
                    }
                }
            }
            continue;
        }

        if (c.height <= 2.0f || hspd == 0.0f)
            continue;

        float previousLeft = previousX + collider.position.x;
        float previousRight = previousLeft + collider.size.x;
        float currentLeft = x + collider.position.x;
        float currentRight = currentLeft + collider.size.x;
        float previousTop = previousY + collider.position.y;
        float previousBottom = previousTop + collider.size.y;
        float currentTop = y + vspd + collider.position.y;
        float currentBottom = currentTop + collider.size.y;

        if (hspd > 0.0f && previousRight <= c.x && currentRight > c.x)
        {
            float contactTime = (c.x - previousRight) / (currentRight - previousRight);
            float contactTop = previousTop + (currentTop - previousTop) * contactTime;
            float contactBottom = previousBottom + (currentBottom - previousBottom) * contactTime;
            bool approachingTop = contactTop < c.y && contactBottom <= c.y + ONE_WAY_TOLERANCE;

            if (contactBottom > c.y && contactTop < c.y + c.height && !approachingTop)
            {
                x = c.x - collider.position.x - collider.size.x;
                hspd = 0.0f;
                setAtWall();
                c.onHit(HitSide::Left, this);
            }
        }
        else if (hspd < 0.0f && previousLeft >= c.x + c.width && currentLeft < c.x + c.width)
        {
            float contactTime = (c.x + c.width - previousLeft) / (currentLeft - previousLeft);
            float contactTop = previousTop + (currentTop - previousTop) * contactTime;
            float contactBottom = previousBottom + (currentBottom - previousBottom) * contactTime;
            bool approachingTop = contactTop < c.y && contactBottom <= c.y + ONE_WAY_TOLERANCE;

            if (contactBottom > c.y && contactTop < c.y + c.height && !approachingTop)
            {
                x = c.x + c.width - collider.position.x;
                hspd = 0.0f;
                setAtWall();
                c.onHit(HitSide::Right, this);
            }
        }
    }

    // Y COLLISIONS
    y += vspd;
    for (auto& o : room->objects)
    {
        Solid* solidPtr = dynamic_cast<Solid*>(o.get());
        if (!solidPtr || !solidPtr->collidable) continue;
        Solid& c = *solidPtr;
        if (c.shape != CollisionShape::Rectangle)
        {
            if (c.points.size() >= 2)
            {
                float colliderCenterX = collider.position.x + collider.size.x * 0.5f;
                float colliderBottom = collider.position.y + collider.size.y;

                float centerX = x + colliderCenterX;
                float previousCenterX = previousX + colliderCenterX;

                float currentBottom = y + colliderBottom;
                float previousBottom = previousY + colliderBottom;
                
                float currentTop = y + collider.position.y;
                float previousTop = previousY + collider.position.y;
                
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
                        if (!hasPreviousFloor ||
                            std::abs(candidate - previousBottom) < std::abs(previousFloor - previousBottom))
                        {
                            previousFloor = candidate;
                            hasPreviousFloor = true;
                        }
                        if (!hasPreviousCeiling ||
                            std::abs(candidate - previousTop) < std::abs(previousCeiling - previousTop))
                        {
                            previousCeiling = candidate;
                            hasPreviousCeiling = true;
                        }
                    }
                }

                bool wasOnPolyline = c.shape == CollisionShape::Polyline && hasPreviousFloor && std::abs(previousBottom - previousFloor) <= 1.0f;

                bool canCatchDownhill =
                    wasOnPolyline && hasCurrentFloor &&
                    currentFloor >= previousFloor &&
                    currentFloor - previousFloor <= ONE_WAY_TOLERANCE;

                if (vspd >= 0.0f && hasCurrentFloor &&
                    (c.shape == CollisionShape::Polyline || hasPreviousFloor) &&
                    (previousBottom <= (hasPreviousFloor ? previousFloor : currentFloor) + (c.shape == CollisionShape::Polyline ? ONE_WAY_TOLERANCE : 0.0f)) &&
                    (currentBottom >= currentFloor || canCatchDownhill))
                {
                    y = currentFloor - collider.position.y - collider.size.y;
                    vspd = 0.0f;
                    onFloor = true;
                    c.onHit(HitSide::Above, this);
                    for (size_t index = 0; index < edgeCount(c); ++index)
                    {
                        const auto& first = c.points[index];
                        const auto& second = c.points[(index + 1) % c.points.size()];
                        float edgeY = 0.0f;
                        if (edgeHeightAtX(first, second, centerX, edgeY) &&
                            std::abs(edgeY - currentFloor) < COLLISION_EPSILON)
                        {
                            slopeAngle = std::atan2(second.y - first.y, second.x - first.x);
                            if (slopeAngle > M_PI * 0.5f)
                                slopeAngle -= M_PI;
                            else if (slopeAngle < -M_PI * 0.5f)
                                slopeAngle += M_PI;
                            isOnSlopeSurface = std::abs(slopeAngle) > COLLISION_EPSILON;
                            break;
                        }
                    }
                }
                else if (c.shape == CollisionShape::Polygon && vspd < 0.0f &&
                    hasCurrentCeiling && hasPreviousCeiling &&
                    previousTop >= previousCeiling
                    && currentTop <= currentCeiling)
                {
                    y = currentCeiling - collider.position.y;
                    vspd = 0.0f;
                    onCeilingHit();
                    c.onHit(HitSide::Below, this);
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
                    onFloor = true;
                    c.onHit(HitSide::Above, this);
                }
                if (vspd >= 0.0f)
                    vspd = 0.0f;
                continue;
            }
            if (vspd > 0.0f)
            {
                // falling
                y -= hit->size.y;
                onFloor = true;
                vspd = 0.0f;
                c.onHit(HitSide::Above, this);
            }
            else if (vspd < 0.0f &&
                previousY + collider.position.y >= c.y + c.height &&
                y + collider.position.y < c.y + c.height)
            {
                // Pass through the top, but collide with the underside.
                y += hit->size.y;
                vspd = 0.0f;
                onCeilingHit();
                c.onHit(HitSide::Below, this);
            }
        }
    }

    if (!onFloor && previousFloor && vspd >= 0.0f)
    {
        float centerX = x + collider.position.x + collider.size.x * 0.5f;
        float feetY = y + collider.position.y + collider.size.y;
        float supportY = 0.0f;
        float supportDistance = 0.0f;
        bool foundSupport = false;
        Solid* supportSolid = nullptr;

        for (auto& o : room->objects)
        {
            Solid* c = dynamic_cast<Solid*>(o.get());
            if (!c || !c->collidable) continue;

            if (c->shape == CollisionShape::Rectangle)
            {
                if (c->width <= 0.0f || centerX < c->x || centerX > c->x + c->width)
                    continue;

                float distance = std::abs(feetY - c->y);
                if (!foundSupport || distance < supportDistance)
                {
                    foundSupport = true;
                    supportDistance = distance;
                    supportY = c->y;
                    supportSolid = c;
                }
                continue;
            }

            for (size_t index = 0; index < edgeCount(*c); ++index)
            {
                float candidateY = 0.0f;
                const auto& first = c->points[index];
                const auto& second = c->points[(index + 1) % c->points.size()];
                if (!edgeHeightAtX(first, second, centerX, candidateY))
                    continue;

                float distance = std::abs(feetY - candidateY);
                if (!foundSupport || distance < supportDistance)
                {
                    foundSupport = true;
                    supportDistance = distance;
                    supportY = candidateY;
                    supportSolid = c;
                }
            }
        }

        if (foundSupport && supportDistance <= ONE_WAY_TOLERANCE)
        {
            y = supportY - collider.position.y - collider.size.y;
            vspd = 0.0f;
            onFloor = true;
            if (supportSolid) supportSolid->onHit(HitSide::Above, this);
        }
    }
}