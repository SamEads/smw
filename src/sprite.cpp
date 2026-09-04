#include <json.hpp>
#include <fstream>
#include "sprite.h"
#include "assets.h"
#include "textures.h"

void Sprite::load(const std::filesystem::path& png, const std::filesystem::path& json)
{
    sprite = std::make_unique<sf::Sprite>(Textures::get(png));

    std::ifstream f(GetAssetDirectory(json));
    nlohmann::ordered_json data = nlohmann::ordered_json::parse(f);

    for (auto& t : data["frames"])
    {
        Frame f;
        f.rect.position = { t["frame"]["x"].get<int>(), t["frame"]["y"].get<int>() };
        f.rect.size = { t["frame"]["w"].get<int>(), t["frame"]["h"].get<int>() };
        frames.push_back(f);
    }

    for (auto& t : data["meta"]["frameTags"])
    {
        std::string animName = t["name"].get<std::string>();
        Animation& a = animations[animName];
        a.from = t["from"].get<int>();
        a.to = t["to"].get<int>();
    }
}

void Sprite::draw(sf::RenderTarget& target, float x, float y)
{
    if (!sprite) return;

    auto& animData = animations[animation];

    int frameCount = animData.to - animData.from + 1;
    float visFrame = animData.from + std::fmod(frame, frameCount);
    sprite->setTextureRect(frames[visFrame].rect);
    sprite->setPosition({ x, y });
    sprite->setOrigin(origin);
    target.draw(*sprite.get());
}

void Sprite::setAnimation(const std::string& anim)
{
    if (!sprite) return;
    animation = anim;
}

void Sprite::setOrigin(float x, float y)
{
    origin.x = x;
    origin.y = y;
}