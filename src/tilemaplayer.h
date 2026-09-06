#pragma once

#include <vector>
#include "gameobject.h"

class TilemapLayerChunk
{
public:
    int x, y;
    int width, height;
    std::vector<int> values;
};

class TilemapLayer : public GameObject
{
public:
    TilemapLayer(Room* room);

public:
    int width, height;
    std::vector<TilemapLayerChunk> chunks;

public:
    void draw(sf::RenderTarget& target) override;
};