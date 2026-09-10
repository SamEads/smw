#pragma once

#include <SFML/System/Vector2.hpp>
#include <vector>
#include "../core/gameobject.h"

class PhysicsEntity;

enum class CollisionShape
{
	Rectangle,
	Polyline,
	Polygon
};

// Which face of a Solid an Actor made contact with.
enum class HitSide
{
	Above,
	Below,
	Left,
	Right
};

// Anything an Actor (PhysicsEntity) can stand on or be blocked by.
// Most Solids are static level geometry: shape/points loaded once from the
// level and never touched again, so the movement machinery below is a no-op
// for them. A Solid that repositions itself in its own step() should call
// resolveMovement() afterward so any riders get carried along and any actors
// it moved into get pushed out of the way.
class Solid : public GameObject
{
public:
	Solid(Room* room);

public:
	float width = 0.0f, height = 0.0f;
	CollisionShape shape = CollisionShape::Rectangle;
	std::vector<sf::Vector2f> points;

	// When false, actors pass straight through this solid (e.g. a block
	// that's been knocked through and hasn't reset yet).
	bool collidable = true;

public:
	ObjectCategory getCategory() const override { return ObjectCategory::COLLISION; }
	bool getWorldBounds(sf::FloatRect& bounds) const override;

	// Called on the solid an Actor's collision resolution made contact with,
	// from the solid's point of view. Default does nothing; override to
	// react (bounce, break, become uncollidable, spawn something, etc).
	virtual void onHit(HitSide side, PhysicsEntity* entity) {}

protected:
	// Carries any Actor resting on top of this solid by this frame's delta,
	// and pushes any Actor this solid's horizontal movement walked into.
	// Call once from step() after updating x/y.
	void resolveMovement();
};
