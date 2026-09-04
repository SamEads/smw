#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

#include "tilemaplayer.h"
#include "collision.h"
#include "player.h"

class Room
{
public:
	std::vector<TilemapLayer> layers;
	std::vector<Collision> collisions;

    float camX, camY;

    Player player;

    void step();
    void draw(sf::RenderTarget& target);
};