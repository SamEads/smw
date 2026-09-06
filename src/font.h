#pragma once

#include <unordered_map>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "assets.h"

class Font
{
public:
    using GlyphMap = std::unordered_map<char, int>;
    enum class Alignment
    {
        LEFT,
        RIGHT
    };

public:
    static Font SMALL;
    static Font POINTS;

public:
    void initialize(std::filesystem::path sprite, int charWidth, int charHeight, const GlyphMap& glyphMap);
    void draw(const std::string& text, sf::RenderTarget& target, float x, float y, sf::Color color = sf::Color::White, Alignment alignment = Alignment::LEFT);

private:
    GlyphMap glyphMap;
    int charWidth, charHeight;
    std::unique_ptr<sf::Sprite> sprite;

};
