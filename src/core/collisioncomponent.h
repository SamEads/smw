#pragma once

#include <SFML/Graphics/Rect.hpp>

struct CollisionComponent
{
    sf::FloatRect localBounds;
    bool solid = true;
};
