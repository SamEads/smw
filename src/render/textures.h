#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <memory>
#include <unordered_map>

class Textures
{
private:
    static std::unordered_map<std::string, std::unique_ptr<sf::Texture>> textures;

public:
    static sf::Texture& get(const std::string& res);
};