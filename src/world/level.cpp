#include "level.h"

#include <fstream>
#include <json.hpp>

#include "assets.h"
#include "backgroundlayer.h"
#include "player.h"
#include "qblock.h"

Level::Level(const std::filesystem::path& mapPath, Game* game) : Room(game)
{
    std::ifstream input(mapPath);
    nlohmann::json map = nlohmann::json::parse(input);

    const std::string backgroundHex = map.value("backgroundcolor", "#000000FF");
    std::string hex = backgroundHex[0] == '#' ? backgroundHex.substr(1) : backgroundHex;
    if (hex.size() == 6)
        hex += "FF";
    const unsigned long colorValue = std::stoul(hex, nullptr, 16);
    bgColor = sf::Color{
        static_cast<std::uint8_t>((colorValue >> 24) & 0xFF),
        static_cast<std::uint8_t>((colorValue >> 16) & 0xFF),
        static_cast<std::uint8_t>((colorValue >> 8) & 0xFF),
        static_cast<std::uint8_t>(colorValue & 0xFF)
    };

    for (auto& layerData : map["layers"])
    {
        if (layerData["type"] == "tilelayer")
        {
            auto layer = std::make_unique<TilemapLayer>(this);
            layer->width = layerData["width"].get<int>();
            layer->height = layerData["height"].get<int>();
            for (auto& chunkData : layerData["chunks"])
            {
                TilemapLayerChunk& chunk = layer->chunks.emplace_back();
                chunk.values = chunkData["data"].get<std::vector<int>>();
                chunk.x = chunkData["x"].get<int>();
                chunk.y = chunkData["y"].get<int>();
                chunk.width = chunkData["width"].get<int>();
                chunk.height = chunkData["height"].get<int>();

                float chunkRight = (chunk.x + chunk.width) * 16;
                if (chunkRight > width)
                {
                    bool found = false;
                    for (int xx = chunk.width - 1; xx >= 0 && !found; --xx)
                    {
                        for (int yy = 0; yy < chunk.height; ++yy)
                        {
                            if (chunk.values[xx + yy * chunk.width] != 0)
                            {
                                width = (chunk.x + xx + 1) * 16;
                                found = true;
                                break;
                            }
                        }
                    }
                }

                float chunkBottom = (chunk.y + chunk.height) * 16;
                if (chunkBottom > height)
                {
                    bool found = false;
                    for (int yy = chunk.height - 1; yy >= 0 && !found; --yy)
                    {
                        for (int xx = 0; xx < chunk.width; ++xx)
                        {
                            if (chunk.values[xx + yy * chunk.width] != 0)
                            {
                                height = (chunk.y + yy + 1) * 16;
                                found = true;
                                break;
                            }
                        }
                    }
                }
            }

            if (layerData.contains("properties"))
            {
                for (auto& property : layerData["properties"])
                {
                    if (property["name"] == "depth")
                        layer->depth = property["value"].get<int>();
                }
            }

            layer->buildChunks();

            addObject(std::move(layer));
        }
        else if (layerData["name"] == "Collisions")
        {
            for (auto& objectData : layerData["objects"])
            {
                auto& collision = collisions.emplace_back();
                collision.x = objectData["x"];
                collision.y = objectData["y"];
                collision.width = objectData.value("width", 0.0f);
                collision.height = objectData.value("height", 0.0f);

                if (objectData.contains("polyline"))
                {
                    collision.shape = CollisionShape::Polyline;
                    for (auto& point : objectData["polyline"])
                    {
                        collision.points.emplace_back(
                            collision.x + point["x"].get<float>(),
                            collision.y + point["y"].get<float>());
                    }
                }
                else if (objectData.contains("polygon"))
                {
                    collision.shape = CollisionShape::Polygon;
                    for (auto& point : objectData["polygon"])
                    {
                        collision.points.emplace_back(
                            collision.x + point["x"].get<float>(),
                            collision.y + point["y"].get<float>());
                    }
                }
                else if (collision.width > 0.0f && collision.height > 0.0f)
                {
                    collision.points = {
                        { collision.x, collision.y },
                        { collision.x + collision.width, collision.y },
                        { collision.x + collision.width, collision.y + collision.height },
                        { collision.x, collision.y + collision.height }
                    };
                }
                else if (collision.width > 0.0f)
                {
                    collision.shape = CollisionShape::Polyline;
                    collision.points = {
                        { collision.x, collision.y },
                        { collision.x + collision.width, collision.y }
                    };
                }
            }
        }
        else if (layerData["type"] == "objectgroup")
        {
            for (auto& objectData : layerData["objects"])
            {
                std::string objectType = objectData.value("type", "");
                std::string objectName = objectData.value("name", "");
                if (objectType == "qblock" || objectName == "qblock" || objectName == "QBlock")
                {
                    addObject(std::make_unique<QBlock>(this,
                        objectData["x"].get<float>(), objectData["y"].get<float>()));
                }
            }
        }
        else if (layerData["type"] == "imagelayer")
        {
            std::filesystem::path imagePath = std::filesystem::path("levels") /
                layerData["image"].get<std::string>();
            auto background = std::make_unique<BackgroundLayer>(this, imagePath);
            if (layerData.contains("parallaxx")) background->parallaxX = layerData["parallaxx"];
            if (layerData.contains("parallaxy")) background->parallaxY = layerData["parallaxy"];
            if (layerData.contains("x")) background->x = layerData["x"];
            if (layerData.contains("y")) background->y = layerData["y"];
            if (layerData.contains("properties"))
            {
                for (auto& property : layerData["properties"])
                {
                    if (property["name"] == "depth")
                        background->depth = property["value"].get<int>();
                }
            }
            addObject(std::move(background));
        }
    }

    auto playerObject = std::make_unique<Player>(this, *game);
    player = playerObject.get();
    addObject(std::move(playerObject));
}
