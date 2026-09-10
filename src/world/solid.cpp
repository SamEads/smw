#include "solid.h"
#include "room.h"
#include "../actors/physicsentity.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float RIDE_TOLERANCE = 2.0f;
}

Solid::Solid(Room* room) : GameObject(room)
{
}

bool Solid::getWorldBounds(sf::FloatRect& bounds) const
{
	if (points.empty())
	{
		bounds = sf::FloatRect({ x, y }, { width, height });
		return true;
	}

	float minX = points.front().x;
	float minY = points.front().y;
	float maxX = minX;
	float maxY = minY;
	for (const auto& point : points)
	{
		minX = std::min(minX, point.x);
		minY = std::min(minY, point.y);
		maxX = std::max(maxX, point.x);
		maxY = std::max(maxY, point.y);
	}

	bounds = sf::FloatRect({ minX, minY }, { maxX - minX, maxY - minY });
	if (bounds.size.x == 0.0f)
	{
		bounds.position.x -= 0.5f;
		bounds.size.x = 1.0f;
	}
	if (bounds.size.y == 0.0f)
	{
		bounds.position.y -= 0.5f;
		bounds.size.y = 1.0f;
	}
	return true;
}

void Solid::resolveMovement()
{
	if (!collidable) return;

	float dx = x - xPrevious;
	float dy = y - yPrevious;
	if (dx == 0.0f && dy == 0.0f)
		return;

	sf::FloatRect bounds;
	if (!getWorldBounds(bounds))
		return;

	sf::FloatRect previousBounds = bounds;
	previousBounds.position -= { dx, dy };

	for (auto& o : room->objects)
	{
		PhysicsEntity* actor = dynamic_cast<PhysicsEntity*>(o.get());
		if (!actor)
			continue;

		sf::FloatRect actorBounds;
		actor->getWorldBounds(actorBounds);

		bool horizontallyAligned =
			actorBounds.position.x + actorBounds.size.x > previousBounds.position.x &&
			actorBounds.position.x < previousBounds.position.x + previousBounds.size.x;
		bool wasRiding = horizontallyAligned && std::abs(
			(actorBounds.position.y + actorBounds.size.y) - previousBounds.position.y) <= RIDE_TOLERANCE;

		if (wasRiding)
		{
			actor->x += dx;
			actor->y += dy;
			actor->groundVelocity = { dx, dy };
			continue;
		}

		if (dx == 0.0f)
			continue;

		auto overlap = bounds.findIntersection(actorBounds);
		if (!overlap.has_value())
			continue;

		if (dx > 0.0f)
			actor->x += (bounds.position.x + bounds.size.x) - actorBounds.position.x;
		else
			actor->x += bounds.position.x - (actorBounds.position.x + actorBounds.size.x);
	}
}
