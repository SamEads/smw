#include "tilemaplayer.h"
#include "textures.h"
#include "room.h"
#include "enums.h"
#include <SFML/Graphics.hpp>

TilemapLayer::TilemapLayer(Room *room) : GameObject(room)
{
}

void TilemapLayer::draw(sf::RenderTarget &target)
{
    sf::Texture& tex = Textures::get("tiles/ground.png");
	sf::Sprite tile(tex);
	int tilesX = tex.getSize().x / 16;

    static float retarded = 0.0f;
    int calls = 0;
    float camX = room->camX;
    float camY = room->camY;
    for (auto& c : chunks)
    {
        int startX = c.x * 16;
        if (startX - GAME_WIDTH > camX) continue;
        if (startX + GAME_WIDTH < camX) continue;
        int startY = c.y * 16;
        if (startY - GAME_HEIGHT > camY) continue;
        if (startY + GAME_HEIGHT < camY) continue;
        int chunkSize = c.values.size();
        for (int i = 0; i < chunkSize; ++i)
        {
            int tileInt = c.values[i];
            if (tileInt == 0) continue;
            tileInt -= 1;
            int posX = i % c.width;
            float screenX = (float)startX + (posX * 16);
            if (screenX < camX - 16) continue;
            if (screenX > camX + GAME_WIDTH) continue;

            int posY = i / c.width;
            float screenY = (float)startY + (posY * 16);
            if (screenY < camY - 16) continue;
            if (screenY > camY + GAME_HEIGHT) continue;

            int texX = tileInt % tilesX;
            int texY = tileInt / tilesX;
            tile.setPosition({ screenX, screenY });
            tile.setTextureRect({ { texX * 16, texY * 16 }, { 16, 16 } });
            target.draw(tile);
            ++calls;
        }
    }
}