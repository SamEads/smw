#pragma once

#define GAME_WIDTH 256
#define GAME_HEIGHT 224

enum class ObjectCategory
{
	WORLD,
	PLAYER,
	ENEMY,
	ITEM,
	GIZMO,
	LIQUID,
	EFFECT,
	COLLISION
};