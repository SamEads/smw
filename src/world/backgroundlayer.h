#pragma once

#include "gameobject.h"
#include "assets.h"
#include <SFML/Graphics/Sprite.hpp>

class BackgroundLayer : public GameObject
{
public:
    BackgroundLayer(Room* room, const std::filesystem::path& imgPath);

public:
    void draw(sf::RenderTarget& target, float interp) override;

public:
    float parallaxX = 0.0f;
    float parallaxY = 0.0f;

private:
    std::unique_ptr<sf::Sprite> sprite;
};