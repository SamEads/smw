#pragma once

#include "gameobject.h"

#include <SFML/Graphics.hpp>
#include <vector>

class TilemapLayerChunk
{
public:
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    std::vector<int> values;

    sf::VertexArray vertices{sf::PrimitiveType::Triangles};

    void build(int tilesX);
};

class TilemapLayer : public GameObject
{
public:
    TilemapLayer(Room* room);

    int width = 0;
    int height = 0;

    std::vector<TilemapLayerChunk> chunks;

    void buildChunks();
    void draw(sf::RenderTarget& target, float interp) override;
};