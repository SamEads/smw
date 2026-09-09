#include <algorithm>
#include "room.h"
#include "enums.h"
#include "mathhelper.h"
#include "textures.h"
#include "player.h"
#include "font.h"
#include "assets.h"

Room::Room(Game* game) : game(game)
{
    timerDecrementer = 45;
    levelTime = 300;
}

std::vector<const Collision*> Room::queryCollisions(const sf::FloatRect& area) const
{
    std::vector<const Collision*> results;
    for (const auto& o : objects)
    {
        if (o->getCategory() != ObjectCategory::COLLISION)
            continue;
        Collision* collision = (Collision*)o.get();
        sf::FloatRect bounds;
        if (collision->points.empty())
        {
            bounds = sf::FloatRect(
                { collision->x, collision->y },
                { collision->width, collision->height });
        }
        else
        {
            float minX = collision->points.front().x;
            float minY = collision->points.front().y;
            float maxX = minX;
            float maxY = minY;
            for (const auto& point : collision->points)
            {
                minX = std::min(minX, point.x);
                minY = std::min(minY, point.y);
                maxX = std::max(maxX, point.x);
                maxY = std::max(maxY, point.y);
            }
            bounds = sf::FloatRect({ minX, minY }, { maxX - minX, maxY - minY });
            if (bounds.size.x == 0.0f)
            {
                bounds.position.x -= 0.5f;
                bounds.size.x = 1.0f;
            }
            if (bounds.size.y == 0.0f)
            {
                bounds.position.y -= 0.5f;
                bounds.size.y = 1.0f;
            }
        }

        if (bounds.findIntersection(area).has_value())
            results.push_back(collision);

    }
    return results;
}

std::vector<GameObject*> Room::queryObjects(const sf::FloatRect& area,
    const GameObject* ignore) const
{
    std::vector<GameObject*> results;
    for (const auto& object : objects)
    {
        if (object.get() == ignore)
            continue;

        sf::FloatRect bounds;
        if (object->getWorldBounds(bounds) && bounds.findIntersection(area).has_value())
            results.push_back(object.get());
    }
    return results;
}

std::vector<GameObject*> Room::queryObjects(const sf::FloatRect& area,
    ObjectCategory category, const GameObject* ignore) const
{
    std::vector<GameObject*> results;
    for (const auto& object : objects)
    {
        if (object.get() == ignore || object->getCategory() != category)
            continue;

        sf::FloatRect bounds;
        if (object->getWorldBounds(bounds) && bounds.findIntersection(area).has_value())
            results.push_back(object.get());
    }
    return results;
}

int x_side = 0;
float destinationY = 0;
bool followingDown = false;
void Room::step()
{
    prevCamX = camX;
    prevCamY = camY;
    float lastcx = internalCamX;
    qblockAnimationFrame += 0.125f;
    if (qblockAnimationFrame >= 4.0f)
        qblockAnimationFrame -= 4.0f;

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
    for (auto& o : objects)
    {
        o->postStep();
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

    if ((player->isOnFloor() && player->vspd == 0.0f) ||
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


    camX = MathHelper::clamp(internalCamX - std::floorf(GAME_WIDTH / 2.0f), 0, width - GAME_WIDTH);
    camY = MathHelper::clamp(internalCamY - std::floorf(GAME_HEIGHT / 2.0f), 0, height - GAME_HEIGHT);
}

void Room::draw(sf::RenderTarget &target, float interp)
{
    sf::RectangleShape rs({ GAME_WIDTH, GAME_HEIGHT });
    rs.setFillColor(bgColor);
    target.draw(rs);

    sf::View gameView(sf::FloatRect{ { 0, 0 }, { (float)GAME_WIDTH, (float)GAME_HEIGHT } });

    float interpCamX = MathHelper::lerp(prevCamX, camX, interp);
    float interpCamY = MathHelper::lerp(prevCamY, camY, interp);
    gameView.setCenter({ std::floorf(interpCamX) + (GAME_WIDTH / 2.0f), std::floorf(interpCamY) + (GAME_HEIGHT / 2.0f) });
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
        obj->draw(target, interp);
    }

    /*
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
    */

    sf::View hudView(sf::FloatRect{ { 0, 0 }, { (float)GAME_WIDTH, (float)GAME_HEIGHT } });
    target.setView(hudView);

    // sf::Sprite hudtemp(Textures::get("sprites/hudtemp.png"));
    // hudtemp.setColor({ 255, 255, 255, 100 });
    // hudtemp.setPosition({ 16, 9 });
    // target.draw(hudtemp);

    sf::Sprite reserveSpr(Textures::get("sprites/hud/reserve.png"));
    const char* characterHud = game && game->playerCharacter == PlayerCharacter::MARIO ?
        "sprites/hud/mario.png" : "sprites/hud/luigi.png";
    sf::Sprite characterSpr(Textures::get(characterHud));
    sf::Sprite timeSpr(Textures::get("sprites/hud/time.png"));
    sf::Sprite coinsSpr(Textures::get("sprites/hud/coins.png"));
    sf::Sprite tapePtsSprite(Textures::get("sprites/hud/tape_points.png"));
    const Game::CharacterData* characterData = game ?
        &game->dataFor(game->playerCharacter) : nullptr;

    float yAnchor = std::floorf(GAME_HEIGHT / 28.0f);
    if (GAME_HEIGHT < 200)
    {
        yAnchor = std::floorf(GAME_HEIGHT / 48.0f);
    }

    reserveSpr.setPosition({ (GAME_WIDTH / 2) - 14, yAnchor });
    target.draw(reserveSpr);

    float leftWeight = std::floorf(GAME_WIDTH / 4.0f) - 48;
    characterSpr.setPosition({ leftWeight, yAnchor + 5 });
    target.draw(characterSpr);
    Font::SMALL.draw("x", target, leftWeight + 8, yAnchor + 14);
    Font::SMALL.draw(std::to_string(characterData ? characterData->lives : 5), target, leftWeight + 32, yAnchor + 14,
        sf::Color::White, Font::Alignment::RIGHT);

    float tapeWeight = std::floorf(GAME_WIDTH / 2.0f) - std::floorf(GAME_WIDTH / 6.0f) + 6;
    if (GAME_WIDTH < 256)
    {
        tapeWeight = std::floorf(GAME_WIDTH / 2.0f) - std::floorf(GAME_WIDTH / 5.7f);
    }
    tapePtsSprite.setPosition({ tapeWeight - 20, yAnchor + 14 });
    Font::SMALL.draw("x", target, tapeWeight + 8 - 20, yAnchor + 14);
    target.draw(tapePtsSprite);
    Font::POINTS.draw(std::to_string(characterData ? characterData->tapeScore : 0), target, tapeWeight + 20, yAnchor + 5,
        sf::Color::White, Font::Alignment::RIGHT);
    
    float timerWeight = std::floorf(GAME_WIDTH / 2.0f) + std::floorf(GAME_WIDTH / 5.0f) - 25;
    if (GAME_WIDTH < 256)
    {
        float timerSpriteSize = timeSpr.getTexture().getSize().x;
        float szHalfTimer = timerSpriteSize / 2.0f;
        timerWeight = std::floorf(GAME_WIDTH / 2.0f) + std::floorf(GAME_WIDTH / 5.7f) - szHalfTimer;
    }
    timeSpr.setPosition({ timerWeight, yAnchor + 6 });
    target.draw(timeSpr);
    Font::SMALL.draw(std::to_string(levelTime), target, timerWeight + 24, yAnchor + 14, { 252, 220, 114 }, Font::Alignment::RIGHT);

    float coinsWeight = GAME_WIDTH - (GAME_WIDTH / 4.0f) + 48;
    coinsSpr.setPosition({ coinsWeight - 40, yAnchor + 6 });
    Font::SMALL.draw("x", target, coinsWeight - 32, yAnchor + 6);
    target.draw(coinsSpr);
    Font::SMALL.draw(std::to_string(characterData ? characterData->coins : 0), target, coinsWeight, yAnchor + 6,
        sf::Color::White, Font::Alignment::RIGHT);

    // Score
    Font::SMALL.draw(std::to_string(characterData ? characterData->score : 0), target,
        coinsWeight, yAnchor + 14, sf::Color::White, Font::Alignment::RIGHT);

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
