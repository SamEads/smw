#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <zlib.h>
#include <json.hpp>

#include "room.h"
#include "sprite.h"
#include "player.h"
#include "enums.h"
#include "assets.h"
#include "keys.h"

int main()
{
	sf::RenderWindow window(sf::VideoMode({ GAME_WIDTH * 3, GAME_HEIGHT * 3 }), "SUPER FUCKING MARIO WORLD!!!!!!!!!!!!! TRANSGENDER" );
	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	sf::RenderTexture t({ GAME_WIDTH, GAME_HEIGHT });

	std::ifstream i(GetAssetDirectory("levels/level0.tmj"));
	nlohmann::json j = nlohmann::json::parse(i);

	Room room;
	for (auto& l : j["layers"])
	{
		if (l["type"] == "tilelayer")
		{
			std::unique_ptr<TilemapLayer> layer = std::make_unique<TilemapLayer>(&room);
			layer->width = l["width"].get<int>();
			layer->height = l["height"].get<int>();
			for (auto& c : l["chunks"])
			{
				TilemapLayerChunk& chunk = layer->chunks.emplace_back();
				chunk.values = 	c["data"].get<std::vector<int>>();
				chunk.x = 		c["x"].get<int>();
				chunk.y = 		c["y"].get<int>();
				chunk.width = 	c["width"].get<int>();
				chunk.height =	c["height"].get<int>();
				if (chunk.width > room.width)
				{
					room.width = chunk.width;
				}
			}
			room.objects.push_back(std::move(layer));
		}
		else if (l["name"] == "Collisions")
		{
			for (auto& c : l["objects"])
			{
				auto& collision = room.collisions.emplace_back();
				collision.x = c["x"];
				collision.y = c["y"];
				collision.width = c["width"];
				collision.height = c["height"];
			}
		}
	}

	{
		std::unique_ptr<Player> player = std::make_unique<Player>(&room);
		room.player = player.get();
		room.objects.push_back(std::move(player));
	}

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
		}
		Keys::update(window.hasFocus());

		room.step();

		t.clear();
		room.draw(t);
		t.display();

		const auto windowSize = window.getSize();
        sf::View view(sf::FloatRect{ { 0, 0 }, { (float)windowSize.x, (float)windowSize.y } });
        view.setCenter({ windowSize.x / 2.0f, windowSize.y / 2.0f });
        window.setView(view);

		window.clear();
		sf::Sprite ss(t.getTexture());
		float gameScaleX = windowSize.x / (float)GAME_WIDTH;
		float gameScaleY = windowSize.y / (float)GAME_HEIGHT;
		float gameScaleMin = std::min(gameScaleX, gameScaleY);
		float flooredGameScaleMin = std::floorf(gameScaleMin);
		if (flooredGameScaleMin != 0.0f)
			gameScaleMin = flooredGameScaleMin;
		ss.setScale({ gameScaleMin, gameScaleMin });
		ss.setOrigin({ GAME_WIDTH / 2.0f, GAME_HEIGHT / 2.0f });
		ss.setPosition({ std::floorf(windowSize.x / 2.0f), std::floorf(windowSize.y / 2.0f) });
		window.draw(ss);
		window.display();
	}
}
