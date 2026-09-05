#include "room.h"
#include "enums.h"
#include "mathhelper.h"
#include "textures.h"
#include<iostream>

void Room::step()
{
    player.step();   
    camX = player.x - 128;

    float width = 0.0f;
    for (auto& l : layers)
    {
        if (l.width > width)
        {
            width = l.width;
        }
    }
    camX = MathHelper::clamp(camX, 0, (width * 16.0f) - GAME_WIDTH);
}

void Room::draw(sf::RenderTarget &target)
{
    sf::View gameView(sf::FloatRect{ { 0, 0 }, { (float)GAME_WIDTH, (float)GAME_HEIGHT } });

    gameView.setCenter({ std::floorf(camX) + (GAME_WIDTH / 2.0f), std::floorf(camY) + (GAME_HEIGHT / 2.0f) });
    target.setView(gameView);

	sf::Texture tileTexture(Textures::get("tiles/ground.png"));
	sf::Sprite tile(tileTexture);
	int tilesX = tileTexture.getSize().x / 16;

    int depth = 0;
    for (auto& l : layers)
    {
        if (depth == 1)
        {
            player.sprite.draw(target, floorf(player.x), floorf(player.y) + 1);
        }
        for (auto& c : l.chunks)
        {
            int startX = c.x * 16;
            int startY = c.y * 16;
            int chunkSize = c.values.size();
            for (int i = 0; i < chunkSize; ++i)
            {
                int tileInt = c.values[i];
                if (tileInt == 0) continue;
                tileInt -= 1;
                int posX = i % c.width;
                int posY = i / c.width;
                int texX = tileInt % tilesX;
                int texY = tileInt / tilesX;
                tile.setPosition({ (float)startX + (posX * 16), (float)startY + (posY * 16) });
                tile.setTextureRect({ { texX * 16, texY * 16 }, { 16, 16 } });
                target.draw(tile);
            }
        }
        depth++;
    }

    /*
    for (auto& c : collisions)
    {
        sf::RectangleShape rs({ c.width - 2, c.height - 2 });
        rs.setPosition({ c.x + 1, c.y + 1 });
        rs.setFillColor(sf::Color { 0, 0, 0, 128 });
        rs.setOutlineColor(sf::Color { 128, 128, 128, 255 });
        rs.setOutlineThickness(1);
        target.draw(rs);
    }
    */

}
