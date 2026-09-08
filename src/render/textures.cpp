#include "textures.h"
#include "assets.h"

std::unordered_map<std::string, std::unique_ptr<sf::Texture>> Textures::textures;

sf::Texture &Textures::get(const std::string& res)
{
    auto it = textures.find(res);
    if (it != textures.end())
        return *(*it).second.get();

#ifdef SFML3
    std::unique_ptr<sf::Texture> tex = std::make_unique<sf::Texture>(GetAssetDirectory(res));
#else
    std::unique_ptr<sf::Texture> tex = std::make_unique<sf::Texture>();
    tex->loadFromFile(GetAssetDirectory(res));
#endif
    sf::Texture* ptr = tex.get();

    // add & return ptr
    textures[res] = std::move(tex);
    return *ptr;
}
