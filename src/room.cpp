#include "room.h"
#include "enums.h"
#include "textures.h"

void Room::step()
{
    player.grav = 0.125f;
    player.vspd += player.grav;
    if (player.vspd > 4.0f)
    {
        player.vspd = 4.0f;
    }
    player.x += player.hspd;
    player.y += player.vspd;
    for (auto& c : collisions)
    {
        sf::FloatRect floatRect({ c.x, c.y }, { c.width, c.height });
        sf::FloatRect fakePlayer({ player.x - 5, player.y - 10 }, { 10, 10 });
        auto test = floatRect.findIntersection(fakePlayer);
        if (test.has_value())
        {
            sf::FloatRect intersection = test.value();
            player.y = floatRect.position.y;
            player.vspd = 0.0f;
        }
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right))
    {
        player.x += 1.5f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left))
    {
        player.x -= 1.5f;
    }
    player.sprite.frame += 0.2f;
    
    camX = player.x - 128;
}

void Room::draw(sf::RenderTarget &target)
{
    sf::View gameView(sf::FloatRect{ { 0, 0 }, { (float)GAME_WIDTH, (float)GAME_HEIGHT } });

    gameView.setCenter({ std::floorf(camX) + (GAME_WIDTH / 2.0f), std::floorf(camY) + (GAME_HEIGHT / 2.0f) });
    target.setView(gameView);

	sf::Texture tileTexture(Textures::get("tiles/ground.png"));
	sf::Sprite tile(tileTexture);
	int tilesX = tileTexture.getSize().x / 16;

    for (auto& l : layers)
    {
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
    }

    for (auto& c : collisions)
    {
        sf::RectangleShape rs({ c.width - 2, c.height - 2 });
        rs.setPosition({ c.x + 1, c.y + 1 });
        rs.setFillColor(sf::Color { 0, 0, 0, 128 });
        rs.setOutlineColor(sf::Color { 128, 128, 128, 255 });
        rs.setOutlineThickness(1);
        target.draw(rs);
    }

    player.sprite.setAnimation("walk");
    player.sprite.draw(target, player.x, player.y + 1);
}
