#include <algorithm>
#include "room.h"
#include "enums.h"
#include "mathhelper.h"
#include "textures.h"
#include "player.h"

void Room::step()
{
    player->step();   
    camX = player->x - 128;
    camX = MathHelper::clamp(camX, 0, width - GAME_WIDTH);
}

void Room::draw(sf::RenderTarget &target)
{
    sf::View gameView(sf::FloatRect{ { 0, 0 }, { (float)GAME_WIDTH, (float)GAME_HEIGHT } });

    gameView.setCenter({ std::floorf(camX) + (GAME_WIDTH / 2.0f), std::floorf(camY) + (GAME_HEIGHT / 2.0f) });
    target.setView(gameView);


    std::vector<GameObject*> sorted;
    for (auto& o : objects)
    {
        sorted.push_back(o.get());
    }
    std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b)
    {
        return a->depth > b->depth; 
    });
    for (auto& obj : sorted)
    {
        obj->draw(target);
    }
}
