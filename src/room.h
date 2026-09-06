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
    Room();

public:
	std::vector<Collision> collisions;
    std::vector<std::unique_ptr<GameObject>> objects;

    float camX, camY;
    int width, height;

    Player* player;

    bool stepping = false;
    int levelTime = 0;

    sf::Color bgColor;

public:
    void step();
    void draw(sf::RenderTarget& target);
    void addObject(std::unique_ptr<GameObject> gameObject);
    void queueFree(GameObject* gameObject);

private:
    int timerDecrementer;
    float internalCamX = 0;
    std::vector<GameObject*> queuedFree;
    std::vector<std::unique_ptr<GameObject>> queuedAdd;
};