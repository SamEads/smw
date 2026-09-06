#include "backgroundlayer.h"
#include "textures.h"
#include "room.h"

BackgroundLayer::BackgroundLayer(Room *room, const std::filesystem::path& imgPath) : GameObject(room)
{
    sf::Texture& texture = Textures::get(imgPath);
    sprite = std::make_unique<sf::Sprite>(texture);
}

void BackgroundLayer::draw(sf::RenderTarget &target)
{
    const float width = static_cast<float>(sprite->getTexture().getSize().x);
    const float height = static_cast<float>(sprite->getTexture().getSize().y);
    const sf::View& view = target.getView();
    const float viewLeft = view.getCenter().x - view.getSize().x / 2.f;
    const float phase = std::fmod(room->camX * parallaxX, width);
    const float x = phase + std::floor((viewLeft - phase) / width) * width;
    const float viewBottom = view.getCenter().y + view.getSize().y / 2.f;
    const float distanceFromBottom = static_cast<float>(room->height) - viewBottom;
    const float y = static_cast<float>(room->height) - height - distanceFromBottom * parallaxY;

    sprite->setPosition({ std::floor(x), std::floor(y) });
    target.draw(*sprite);
    sprite->setPosition({ std::floor(x + width), std::floor(y) });
    target.draw(*sprite);
}
