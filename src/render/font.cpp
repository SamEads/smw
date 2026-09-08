#include "font.h"
#include "textures.h"

Font Font::SMALL;
Font Font::POINTS;

void Font::initialize(std::filesystem::path sprite, int charWidth, int charHeight, const GlyphMap& glyphMap)
{
    this->charWidth = charWidth;
    this->charHeight = charHeight;
    this->glyphMap = glyphMap;

    sf::Texture& texture = Textures::get(sprite);
    this->sprite = std::make_unique<sf::Sprite>(texture);
    this->sprite->setTextureRect({ { 0, 0 }, { charWidth, charHeight } });
}

void Font::draw(const std::string &text, sf::RenderTarget &target, float x, float y, sf::Color color, Alignment alignment)
{
    sf::Sprite& sprite = *this->sprite.get();
    sprite.setColor(color);

    sf::IntRect rect = sprite.getTextureRect();

    auto calculateOffset = [&](int curTextPos = 0)
    {
        if (alignment == Alignment::RIGHT)
        {
            int offset = 0;
            int firstOf = text.find('\n', 0);
            if (firstOf == std::string::npos)
            {
                firstOf = text.length();
            }
            int len = (firstOf - curTextPos);
            return -len * charWidth;
        }
        return 0;
    };
    float xPos = calculateOffset(), yPos = 0;
#ifdef SFML3
    int texWidth = sprite.getTexture().getSize().x;
#else
    int texWidth = sprite.getTexture()->getSize().x;
#endif
    int charsWidth = texWidth / charWidth;
    for (int i = 0; i < text.length(); ++i)
    {
        char character = text[i];
        if (character == '\n')
        {
            xPos = 0;
            yPos += charHeight;
            continue;
        }
        if (character != ' ')
        {
            int mapPos = glyphMap[character];
#ifdef SFML3
            rect.position.x = (mapPos % charsWidth) * charWidth;
            rect.position.y = (mapPos / charsWidth) * charHeight;
#else
            rect.left = (mapPos % charsWidth) * charWidth;
            rect.top = (mapPos / charsWidth) * charHeight;
#endif
            sprite.setTextureRect(rect);

            sprite.setPosition({ x + xPos, y + yPos });
            target.draw(sprite);
        }
        xPos += charWidth;
    }
}
