#pragma once

#include <SFML/System/Vector2.hpp>
#include <vector>
#include <functional>
#include "../core/gameobject.h"

enum class CollisionShape
{
	Rectangle,
	Polyline,
	Polygon
};

class Collision : public GameObject
{
public:
	float width = 0.0f, height = 0.0f;
	std::function<void()> onCollidedFromBelow;
	std::function<void()> onCollidedFromAbove;
	CollisionShape shape = CollisionShape::Rectangle;
	std::vector<sf::Vector2f> points;
};