#include <algorithm>
#include "room.h"
#include "enums.h"
#include "mathhelper.h"
#include "textures.h"
#include "player.h"
#include "font.h"
#include "assets.h"
#include<iostream>
Room::Room()
{
    timerDecrementer = 45;
    levelTime = 300;
}

int x_side = 0;
void Room::step()
{
    timerDecrementer--;
    if (timerDecrementer == 0)
    {
        timerDecrementer = 45;
        if (levelTime > 0)
            levelTime--;
    }
    for (auto& o : objects)
    {
        o->xPrevious = o->x;
        o->yPrevious = o->y;
    }
    stepping = true;
    for (auto& o : objects)
    {
        o->step();   
    }
    stepping = false;
    for (auto& o : queuedAdd)
    {
        objects.push_back(std::move(o));
    }
    queuedAdd.clear();
    for (auto& o : queuedFree)
    {
        auto find = std::find_if(objects.begin(), objects.end(), [&o](const auto& check)
        {
            return check.get() == o; 
        });
        if (find != objects.end())
        {
            objects.erase(find);
        }
    }
    queuedFree.clear();
    // Player moved far enough left to flip the camera bias.
    if (player->x < internalCamX - 40.0f)
        x_side = -1;
    // Player moved far enough right to flip the camera bias.
    else if (player->x > internalCamX + 40.0f)
        x_side = 1;

    if (x_side == 1 && player->x > internalCamX - 16.0f)
    {
        float spdmod = 0.0f;

        if (player->hspd > 0.0f)
            spdmod = std::abs(player->hspd);

        if (player->x > internalCamX - (16.0f - 2.0f - spdmod))
            internalCamX += 2.0f + player->hspd;
        else
            internalCamX = player->x + 16.0f;

        if (player->hspd == 0.0f)
            internalCamX += player->x - player->xPrevious;
    }
    else
    {
        float spdmod = 0.0f;

        if (player->hspd < 0.0f)
            spdmod = std::abs(player->hspd);

        if (x_side == -1 && player->x < internalCamX + 16.0f)
        {
            if (player->x < internalCamX + (16.0f - 2.0f - spdmod))
                internalCamX -= 2.0f - player->hspd;
            else
                internalCamX = player->x - 16.0f;
        }

        if (player->hspd == 0.0f)
            internalCamX += player->x - player->xPrevious;
    }

    camX = MathHelper::clamp(internalCamX - 128, 0, width - GAME_WIDTH);
}

void Room::draw(sf::RenderTarget &target)
{
    sf::RectangleShape rs({ GAME_WIDTH, GAME_HEIGHT });
    rs.setFillColor(bgColor);
    target.draw(rs);

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

    sf::View hudView(sf::FloatRect{ { 0, 0 }, { (float)GAME_WIDTH, (float)GAME_HEIGHT } });
    target.setView(hudView);

    // sf::Sprite hudtemp(Textures::get("sprites/hudtemp.png"));
    // hudtemp.setColor({ 255, 255, 255, 100 });
    // hudtemp.setPosition({ 16, 9 });
    // target.draw(hudtemp);

    sf::Sprite reserveSpr(Textures::get("sprites/hud/reserve.png"));
    sf::Sprite luigiSpr(Textures::get("sprites/hud/luigi.png"));
    sf::Sprite timeSpr(Textures::get("sprites/hud/time.png"));
    sf::Sprite coinsSpr(Textures::get("sprites/hud/coins.png"));
    sf::Sprite tapePtsSprite(Textures::get("sprites/hud/tape_points.png"));

    reserveSpr.setPosition({ (GAME_WIDTH / 2) - 14, 9 });
    target.draw(reserveSpr);

    luigiSpr.setPosition({ 16, 15 });
    target.draw(luigiSpr);
    Font::SMALL.draw("x", target, 24, 23);
    Font::SMALL.draw("5", target, 48, 23, sf::Color::White, Font::Alignment::RIGHT);

    tapePtsSprite.setPosition({ 72, 23 });
    Font::SMALL.draw("x", target, 80, 23);
    target.draw(tapePtsSprite);
    Font::POINTS.draw("0", target, 112, 14, sf::Color::White, Font::Alignment::RIGHT);
    
    timeSpr.setPosition({ 152, 15 });
    target.draw(timeSpr);
    Font::SMALL.draw(std::to_string(levelTime), target, 176, 23, sf::Color::White, Font::Alignment::RIGHT);

    coinsSpr.setPosition({ 200, 15 });
    Font::SMALL.draw("x", target, 208, 15);
    target.draw(coinsSpr);
    Font::SMALL.draw("0", target, 240, 15, sf::Color::White, Font::Alignment::RIGHT);

    // Score
    Font::SMALL.draw("0", target, 184 + (8 * 7), 23, sf::Color::White, Font::Alignment::RIGHT);
}

void Room::addObject(std::unique_ptr<GameObject> gameObject)
{
    gameObject->xPrevious = gameObject->x;
    gameObject->yPrevious = gameObject->y;
    if (stepping)
    {
        queuedAdd.push_back(std::move(gameObject));
    }
    else
    {
        objects.push_back(std::move(gameObject));
    }
}

void Room::queueFree(GameObject* gameObject)
{
    queuedFree.push_back(gameObject);
}
