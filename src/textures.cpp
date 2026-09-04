#include "textures.h"
#include "assets.h"

std::unordered_map<std::string, std::unique_ptr<sf::Texture>> Textures::textures;

sf::Texture &Textures::get(const std::string& res)
{
    auto it = textures.find(res);
    if (it != textures.end())
        return *(*it).second.get();

    std::unique_ptr<sf::Texture> tex = std::make_unique<sf::Texture>(GetAssetDirectory(res));
    sf::Texture* ptr = tex.get();

    // add & return ptr
    textures[res] = std::move(tex);
    return *ptr;
}
