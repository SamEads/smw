#include <cmath>
#include "backgroundlayer.h"
#include "textures.h"
#include "../core/mathhelper.h"
#include "room.h"

BackgroundLayer::BackgroundLayer(Room *room, const std::filesystem::path& imgPath) : GameObject(room)
{
    sf::Texture& texture = Textures::get(imgPath);
    sprite = std::make_unique<sf::Sprite>(texture);
}

void BackgroundLayer::draw(sf::RenderTarget &target, float interp)
{
    const auto textureSize = sprite->getTexture().getSize();
    const float camX = MathHelper::lerp(room->prevCamX, room->camX, interp);
    const float camY = MathHelper::lerp(room->prevCamY, room->camY, interp);
    const float width = static_cast<float>(textureSize.x);
    const float height = static_cast<float>(textureSize.y);
    const sf::View& view = target.getView();
    const float viewLeft = view.getCenter().x - view.getSize().x / 2.f;
    const float phase = std::fmod(camX * parallaxX, width);
    const float x = phase + std::floorf((viewLeft - phase) / width) * width;
    const float viewBottom = view.getCenter().y + view.getSize().y / 2.f;
    const float distanceFromBottom = static_cast<float>(room->height) - viewBottom;
    const float y = static_cast<float>(room->height) - height - distanceFromBottom * parallaxY;

    sprite->setPosition({ (x), (y) });
    target.draw(*sprite);
    sprite->setPosition({ (x + width), (y) });
    target.draw(*sprite);
}
