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
    previousFloor = onFloor;
    onFloor = false;
    atWall = false;
    isOnSlopeSurface = false;
    slopeAngle = 0.0f;

    x += hspd;
    for (auto& o : room->objects)
    {
        if (o->category != ObjectCategory::Collision) continue;
        Collision& c = *(Collision*)o.get();
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
                    }
                    else if (hspd < 0.0f && previousLeft >= first.x && currentLeft < first.x)
                    {
                        x = first.x - collider.position.x;
                        hspd = 0.0f;
                        setAtWall();
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
            bool approachingTop = contactTop < c.y &&
                contactBottom <= c.y + ONE_WAY_TOLERANCE;
            if (contactBottom > c.y && contactTop < c.y + c.height && !approachingTop)
            {
                x = c.x - collider.position.x - collider.size.x;
                hspd = 0.0f;
                setAtWall();
            }
        }
        else if (hspd < 0.0f && previousLeft >= c.x + c.width && currentLeft < c.x + c.width)
        {
            float contactTime = (c.x + c.width - previousLeft) / (currentLeft - previousLeft);
            float contactTop = previousTop + (currentTop - previousTop) * contactTime;
            float contactBottom = previousBottom + (currentBottom - previousBottom) * contactTime;
            bool approachingTop = contactTop < c.y &&
                contactBottom <= c.y + ONE_WAY_TOLERANCE;
            if (contactBottom > c.y && contactTop < c.y + c.height && !approachingTop)
            {
                x = c.x + c.width - collider.position.x;
                hspd = 0.0f;
                setAtWall();
            }
        }
    }

    y += vspd;

    for (auto& o : room->objects)
    {
        if (o->category != ObjectCategory::Collision) continue;
        Collision& c = *(Collision*)o.get();
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
                    onFloor = true;
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
                    c.onCollidedFromBelow();
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
            }
            else if (vspd < 0.0f &&
                previousY + collider.position.y >= c.y + c.height &&
                y + collider.position.y < c.y + c.height)
            {
                // Pass through the top, but collide with the underside.
                y += hit->size.y;
                vspd = 0.0f;
                onCeilingHit();
                c.onCollidedFromBelow();
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

        for (auto& o : room->objects)
        {
            if (o->category != ObjectCategory::Collision) continue;
            Collision& c = *(Collision*)o.get();
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
            onFloor = true;
        }
    }

    if (onFloor)
        airborneFrames = 0;
    else
        airborneFrames++;
}

/*
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

namespace sf
{
    FloatRect findIntersection(const FloatRect& me, const FloatRect& rectangle)
    {
        // Not using 'std::min' and 'std::max' to avoid depending on '<algorithm>'
        const auto min = [](float a, float b) { return (a < b) ? a : b; };
        const auto max = [](float a, float b) { return (a < b) ? b : a; };

        // Rectangles with negative dimensions are allowed, so we must handle them correctly

        // Compute the min and max of the first rectangle on both axes
        const float r1MinX = min(me.left, static_cast<float>(me.left + me.width));
        const float r1MaxX = max(me.left, static_cast<float>(me.left + me.width));
        const float r1MinY = min(me.top, static_cast<float>(me.top + me.height));
        const float r1MaxY = max(me.top, static_cast<float>(me.top + me.height));

        // Compute the min and max of the second rectangle on both axes
        const float r2MinX = min(rectangle.left, static_cast<float>(rectangle.left + rectangle.width));
        const float r2MaxX = max(rectangle.left, static_cast<float>(rectangle.left + rectangle.width));
        const float r2MinY = min(rectangle.top, static_cast<float>(rectangle.top + rectangle.height));
        const float r2MaxY = max(rectangle.top, static_cast<float>(rectangle.top + rectangle.height));

        // Compute the intersection boundaries
        const float interLeft   = max(r1MinX, r2MinX);
        const float interTop    = max(r1MinY, r2MinY);
        const float interRight  = min(r1MaxX, r2MaxX);
        const float interBottom = min(r1MaxY, r2MaxY);

        return FloatRect({interLeft, interTop}, {interRight - interLeft, interBottom - interTop});
    }
}

bool PhysicsEntity::getWorldBounds(sf::FloatRect& bounds) const
{
    bounds = collider;
#ifdef SFML3
    bounds.position += { x, y };
#else
    bounds.left += x;
    bounds.top += y;
#endif
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
    previousFloor = onFloor;
    onFloor = false;
    atWall = false;
    isOnSlopeSurface = false;
    slopeAngle = 0.0f;

    x += hspd;
    for (auto& c : room->collisions)
    {
        if (c.shape != CollisionShape::Rectangle)
        {
            if (c.shape == CollisionShape::Polygon)
            {
                float previousLeft = previousX + collider.left;
                float previousRight = previousLeft + collider.width;
                float currentLeft = x + collider.left;
                float currentRight = currentLeft + collider.width;
                float top = y + collider.top;
                float bottom = top + collider.height;

                for (size_t index = 0; index < edgeCount(c); ++index)
                {
                    const auto& first = c.points[index];
                    const auto& second = c.points[(index + 1) % c.points.size()];
                    if (std::abs(second.x - first.x) >= COLLISION_EPSILON ||
                        !solidVerticalEdge(first, second, top, bottom))
                        continue;

                    if (hspd > 0.0f && previousRight <= first.x && currentRight > first.x)
                    {
                        x = first.x - collider.left - collider.width;
                        hspd = 0.0f;
                        setAtWall();
                    }
                    else if (hspd < 0.0f && previousLeft >= first.x && currentLeft < first.x)
                    {
                        x = first.x - collider.left;
                        hspd = 0.0f;
                        setAtWall();
                    }
                }
            }
            continue;
        }

        if (c.height <= 2.0f || hspd == 0.0f)
            continue;

        float previousLeft = previousX + collider.left;
        float previousRight = previousLeft + collider.width;
        float currentLeft = x + collider.left;
        float currentRight = currentLeft + collider.width;
        float previousTop = previousY + collider.top;
        float previousBottom = previousTop + collider.height;
        float currentTop = y + vspd + collider.top;
        float currentBottom = currentTop + collider.height;

        if (hspd > 0.0f && previousRight <= c.x && currentRight > c.x)
        {
            float contactTime = (c.x - previousRight) / (currentRight - previousRight);
            float contactTop = previousTop + (currentTop - previousTop) * contactTime;
            float contactBottom = previousBottom + (currentBottom - previousBottom) * contactTime;
            bool approachingTop = contactTop < c.y &&
                contactBottom <= c.y + ONE_WAY_TOLERANCE;
            if (contactBottom > c.y && contactTop < c.y + c.height && !approachingTop)
            {
                x = c.x - collider.left - collider.width;
                hspd = 0.0f;
                setAtWall();
            }
        }
        else if (hspd < 0.0f && previousLeft >= c.x + c.width && currentLeft < c.x + c.width)
        {
            float contactTime = (c.x + c.width - previousLeft) / (currentLeft - previousLeft);
            float contactTop = previousTop + (currentTop - previousTop) * contactTime;
            float contactBottom = previousBottom + (currentBottom - previousBottom) * contactTime;
            bool approachingTop = contactTop < c.y &&
                contactBottom <= c.y + ONE_WAY_TOLERANCE;
            if (contactBottom > c.y && contactTop < c.y + c.height && !approachingTop)
            {
                x = c.x + c.width - collider.left;
                hspd = 0.0f;
                setAtWall();
            }
        }
    }

    y += vspd;

    for (auto& c : room->collisions)
    {
        if (c.shape != CollisionShape::Rectangle)
        {
            if (c.points.size() >= 2)
            {
                float centerX = x + collider.left + collider.width * 0.5f;
                float previousCenterX = previousX + collider.left + collider.width * 0.5f;
                float previousBottom = previousY + collider.top + collider.height;
                float currentBottom = y + collider.top + collider.height;
                float previousTop = previousY + collider.top;
                float currentTop = y + collider.top;
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
                    y = currentFloor - collider.top - collider.height;
                    vspd = 0.0f;
                    onFloor = true;
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
                    y = currentCeiling - collider.top;
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
            playerRect.top += playerRect.height - 5;
            playerRect.height = 5;
        }
        playerRect.left += x, playerRect.top += y;

        if (wall.intersects(playerRect))
        {
            auto hit = findIntersection(wall, playerRect);
            if (oneWay)
            {
                if (vspd >= 0.0f)
                {
                    y -= hit.height;
                    onFloor = true;
                }
                if (vspd >= 0.0f)
                    vspd = 0.0f;
                continue;
            }
            if (vspd > 0.0f)
            {
                // falling
                y -= hit.height;
                onFloor = true;
                vspd = 0.0f;
            }
            else if (vspd < 0.0f &&
                previousY + collider.top >= c.y + c.height &&
                y + collider.top < c.y + c.height)
            {
                // Pass through the top, but collide with the underside.
                y += hit.height;
                vspd = 0.0f;
                onCeilingHit();
            }
        }
    }

    if (!onFloor && previousFloor && vspd >= 0.0f)
    {
        float centerX = x + collider.left + collider.width * 0.5f;
        float feetY = y + collider.top + collider.height;
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
            y = supportY - collider.top - collider.height;
            vspd = 0.0f;
            onFloor = true;
        }
    }

    if (onFloor)
        airborneFrames = 0;
    else
        airborneFrames++;
}
*/