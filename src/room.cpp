#include <algorithm>
#include "room.h"
#include "enums.h"
#include "mathhelper.h"
#include "textures.h"
#include "player.h"
#include "font.h"
#include "assets.h"

Room::Room()
{
    timerDecrementer = 45;
    levelTime = 300;
}

int x_side = 0;
float destinationY = 0;
bool followingDown = false;
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

    constexpr float followDist = 14.0f;
    if (x_side == 1 && player->x > internalCamX - followDist)
    {
        float spdmod = 0.0f;

        if (player->hspd > 0.0f)
            spdmod = std::abs(player->hspd);

        if (player->x > internalCamX - (followDist - 2.0f - spdmod))
            internalCamX += 2.0f + player->hspd;
        else
            internalCamX = player->x + followDist;

        if (player->hspd == 0.0f)
            internalCamX += player->x - player->xPrevious;
    }
    else
    {
        float spdmod = 0.0f;

        if (player->hspd < 0.0f)
            spdmod = std::abs(player->hspd);

        if (x_side == -1 && player->x < internalCamX + followDist)
        {
            if (player->x < internalCamX + (followDist - 2.0f - spdmod))
                internalCamX -= 2.0f - player->hspd;
            else
                internalCamX = player->x - followDist;
        }

        if (player->hspd == 0.0f)
            internalCamX += player->x - player->xPrevious;
    }

    if ((player->isOnFloor && player->vspd == 0.0f) ||
        (player->jumping && player->isPMeterFull()))
    {
        destinationY = player->y;
    }

    if (destinationY < internalCamY && player->y <= internalCamY)
    {
        if (internalCamY > destinationY + 4.0f)
            internalCamY -= 4.0f;
        else
            internalCamY = destinationY;
    }
    if (player->y > internalCamY + 32.0f && !followingDown)
    {
        if (player->vspd >= 0.0f)
            followingDown = true;
    }

    if (followingDown)
    {
        destinationY = internalCamY;

        if (player->vspd < 0.0f)
        {
            followingDown = false;
        }
        else
        {
            internalCamY = player->y - 32.0f;
        }
    }


    camX = MathHelper::clamp(internalCamX - 128, 0, width - GAME_WIDTH);
    camY = MathHelper::clamp(internalCamY - 112, 0, height - GAME_HEIGHT);
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

    for (const auto& collision : collisions)
    {
        if (collision.shape == CollisionShape::Rectangle || collision.points.size() < 2)
            continue;

        size_t edgeCount = collision.shape == CollisionShape::Polygon ?
            collision.points.size() : collision.points.size() - 1;
        sf::VertexArray lines(sf::PrimitiveType::Lines, edgeCount * 2);
        for (size_t index = 0; index < edgeCount; ++index)
        {
            lines[index * 2].position = collision.points[index];
            lines[index * 2 + 1].position = collision.points[(index + 1) % collision.points.size()];
            lines[index * 2].color = sf::Color::Red;
            lines[index * 2 + 1].color = sf::Color::Red;
        }
        target.draw(lines);
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
    Font::SMALL.draw(std::to_string(levelTime), target, 176, 23, { 252, 220, 114 }, Font::Alignment::RIGHT);

    coinsSpr.setPosition({ 200, 15 });
    Font::SMALL.draw("x", target, 208, 15);
    target.draw(coinsSpr);
    Font::SMALL.draw("0", target, 240, 15, sf::Color::White, Font::Alignment::RIGHT);

    // Score
    Font::SMALL.draw("0", target, 184 + (8 * 7), 23, sf::Color::White, Font::Alignment::RIGHT);

    // Font::SMALL.draw(std::to_string((int)width) + "," + std::to_string((int)height), target, GAME_WIDTH - 4, 4,  sf::Color::White, Font::Alignment::RIGHT);
    // Font::SMALL.draw(std::to_string((int)player->x) + "," + std::to_string((int)player->y), target, 4, 4);
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
