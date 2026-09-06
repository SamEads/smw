#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

#include "tilemaplayer.h"
#include "collision.h"
#include "gameobject.h"

class Player;

class Room
{
public:
	std::vector<Collision> collisions;
    std::vector<std::unique_ptr<GameObject>> objects;

    float camX, camY;
    int width, height;

    Player* player;

    void step();
    void draw(sf::RenderTarget& target);
};