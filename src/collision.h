#pragma once

#include <SFML/System/Vector2.hpp>
#include <vector>

enum class CollisionShape
{
	Rectangle,
	Polyline,
	Polygon
};

class Collision
{
public:
	float x = 0.0f, y = 0.0f;
	float width = 0.0f, height = 0.0f;
	CollisionShape shape = CollisionShape::Rectangle;
	std::vector<sf::Vector2f> points;
};