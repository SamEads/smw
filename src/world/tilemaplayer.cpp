#include "tilemaplayer.h"
#include "textures.h"
#include "room.h"
#include "enums.h"

#include <algorithm>
#include <SFML/Graphics.hpp>

constexpr int TILE_SIZE = 16;

void TilemapLayerChunk::build(int tilesX)
{
    vertices.clear();

    // 6 vertices per visible tile
    vertices.resize(
        std::count_if(values.begin(), values.end(),
            [](int value)
            {
                return value != 0;
            }) * 6
    );

    std::size_t vertexIndex = 0;

    for (int i = 0; i < static_cast<int>(values.size()); ++i)
    {
        int tile = values[i];

        if (tile == 0)
            continue;

        --tile;

        const int tileX = i % width;
        const int tileY = i / width;

        const float worldX = static_cast<float>((x + tileX) * TILE_SIZE);
        const float worldY = static_cast<float>((y + tileY) * TILE_SIZE);

        const int texX = tile % tilesX;
        const int texY = tile / tilesX;

        const float u = static_cast<float>(texX * TILE_SIZE);
        const float v = static_cast<float>(texY * TILE_SIZE);

        auto* quad = &vertices[vertexIndex];

        // Triangle 1
        quad[0].position = { worldX,             worldY };
        quad[1].position = { worldX + TILE_SIZE, worldY };
        quad[2].position = { worldX + TILE_SIZE, worldY + TILE_SIZE };

        // Triangle 2
        quad[3].position = { worldX,             worldY };
        quad[4].position = { worldX + TILE_SIZE, worldY + TILE_SIZE };
        quad[5].position = { worldX,             worldY + TILE_SIZE };

        quad[0].texCoords = { u,             v };
        quad[1].texCoords = { u + TILE_SIZE, v };
        quad[2].texCoords = { u + TILE_SIZE, v + TILE_SIZE };

        quad[3].texCoords = { u,             v };
        quad[4].texCoords = { u + TILE_SIZE, v + TILE_SIZE };
        quad[5].texCoords = { u,             v + TILE_SIZE };

        vertexIndex += 6;
    }
}

TilemapLayer::TilemapLayer(Room* room)
    : GameObject(room)
{
}

void TilemapLayer::buildChunks()
{
    const sf::Texture& tex = Textures::get("tiles/ground.png");
    const int tilesX = static_cast<int>(tex.getSize().x) / TILE_SIZE;

    for (auto& chunk : chunks)
        chunk.build(tilesX);
}

void TilemapLayer::draw(sf::RenderTarget& target, float interp)
{
    const sf::Texture& tex = Textures::get("tiles/ground.png");

    const float camX = room->camX;
    const float camY = room->camY;

    sf::RenderStates states;
    states.texture = &tex;

    for (const auto& chunk : chunks)
    {
        const float left   = static_cast<float>(chunk.x * TILE_SIZE);
        const float top    = static_cast<float>(chunk.y * TILE_SIZE);
        const float right  = left + chunk.width * TILE_SIZE;
        const float bottom = top + chunk.height * TILE_SIZE;

        if (right < camX - 16)
            continue;

        if (left > camX + GAME_WIDTH + 16)
            continue;

        if (bottom < camY - 16)
            continue;

        if (top > camY + GAME_HEIGHT + 16)
            continue;

        target.draw(chunk.vertices, states);
    }
}