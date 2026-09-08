#pragma once

#define GAME_WIDTH 256
#define GAME_HEIGHT 224

enum class ObjectCategory
{
	World,
	Player,
	Enemy,
	Item,
	Gizmo,
	Liquid,
	Platform,
	Effect,
	Collision
};