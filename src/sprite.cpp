#include <json.hpp>
#include <fstream>
#include <unordered_map>
#include "sprite.h"
#include "assets.h"
#include "textures.h"

void Sprite::load(const std::filesystem::path& png, const std::filesystem::path& json)
{
    static std::unordered_map<std::string, std::shared_ptr<const Definition>> definitions;

    std::string definitionKey = GetAssetDirectory(json).lexically_normal().generic_string();
    auto definitionIt = definitions.find(definitionKey);
    if (definitionIt == definitions.end())
    {
        auto parsedDefinition = std::make_shared<Definition>();
        std::ifstream f(GetAssetDirectory(json));
        nlohmann::ordered_json data = nlohmann::ordered_json::parse(f);

        for (auto& t : data["frames"])
        {
            Frame frame;
            frame.rect.position = { t["frame"]["x"].get<int>(), t["frame"]["y"].get<int>() };
            frame.rect.size = { t["frame"]["w"].get<int>(), t["frame"]["h"].get<int>() };
            if (frame.rect.size.x > parsedDefinition->width) parsedDefinition->width = frame.rect.size.x;
            if (frame.rect.size.y > parsedDefinition->height) parsedDefinition->height = frame.rect.size.y;
            parsedDefinition->frames.push_back(frame);
        }

        auto& frameTags = data["meta"]["frameTags"];
        if (frameTags.size() != 0)
        {
            parsedDefinition->hasAnimations = true;
            for (auto& t : frameTags)
            {
                std::string animName = t["name"].get<std::string>();
                Animation& animation = parsedDefinition->animations[animName];
                animation.from = t["from"].get<int>();
                animation.to = t["to"].get<int>();
            }
        }

        definitionIt = definitions.emplace(definitionKey, std::move(parsedDefinition)).first;
    }

    definition = definitionIt->second;
    sprite = std::make_unique<sf::Sprite>(Textures::get(png));
    animation.clear();
    frame = 0.0f;
    width = definition->width;
    height = definition->height;
}

void Sprite::draw(sf::RenderTarget& target, float x, float y)
{
    if (!sprite) return;

    if (definition->hasAnimations)
    {
        auto& animData = definition->animations.at(animation);
        int frameCount = animData.to - animData.from + 1;
        float visFrame = animData.from + std::fmod(frame, frameCount);
        sprite->setTextureRect(definition->frames[visFrame].rect);
    }
    else
    {
        float visFrame = (int)frame * width;
        sprite->setTextureRect({ { visFrame, 0 }, { width, height } });
    }
    sprite->setPosition({ x, y });
    sprite->setOrigin(origin);
    sprite->setScale({ (flipX) ? -1.0f : 1.0f, (flipY) ? -1.0f : 1.0f });
    target.draw(*sprite.get());
}

void Sprite::play(const std::string& anim)
{
    if (!sprite) return;
    animation = anim;
}

void Sprite::setOrigin(float x, float y)
{
    origin.x = x;
    origin.y = y;
}

int Sprite::getFrameCount()
{
    if (!definition->hasAnimations)
    {
        return definition->frames.size();
    }
    auto& animData = definition->animations.at(animation);
    int frameCount = animData.to - animData.from + 1;
    return frameCount;
}
