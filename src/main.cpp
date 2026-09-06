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
#include "font.h"

int main()
{
	sf::RenderWindow window(sf::VideoMode({ GAME_WIDTH, GAME_HEIGHT }), "SUPER FUCKING MARIO WORLD!!!!!!!!!!!!! TRANSGENDER" );
	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	sf::RenderTexture t({ GAME_WIDTH, GAME_HEIGHT });

	std::ifstream i(GetAssetDirectory("levels/level0.tmj"));
	nlohmann::json j = nlohmann::json::parse(i);

	Room room;

	Font::SMALL.initialize("sprites/hud/small_font.png", 8, 8,
	{
		{ 'A', 0 },  { 'B', 1 },  { 'C', 2 },
		{ 'D', 3 },  { 'E', 4 },  { 'F', 5 },
		{ 'G', 6 },  { 'H', 7 },  { 'I', 8 },
		{ 'J', 9 },  { 'K', 10 }, { 'L', 11 },
		{ 'M', 12 }, { 'N', 13 }, { 'O', 14 },
		{ 'P', 15 }, { 'Q', 16 }, { 'R', 17 },
		{ 'S', 18 }, { 'T', 19 }, { 'U', 20 },
		{ 'V', 21 }, { 'W', 22 }, { 'X', 23 },
		{ 'Y', 24 }, { 'Z', 25 }, { '0', 26 },
		{ '1', 27 }, { '2', 28 }, { '3', 29 },
		{ '4', 30 }, { '5', 31 }, { '6', 32 },
		{ '7', 33 }, { '8', 34 }, { '9', 35 },
		{ '.', 36 }, { ',', 37 }, { '-', 38 },
		{ '!', 39 }, { '=', 40 }, { ':', 41 },
		{ '\'', 42 },{ '\"', 43 },{ 'x', 44 },
	});
	Font::POINTS.initialize("sprites/hud/points_font.png", 8, 16,
	{
		{ '0', 0 }, { '1', 1 }, { '2', 2 },
		{ '3', 3 }, { '4', 4 }, { '5', 5 },
		{ '6', 6 }, { '7', 7 }, { '8', 8 },
		{ '9', 9 },
	});

	const std::string backgroundHex = j.value("backgroundcolor", "#000000FF");
	std::string hex = backgroundHex[0] == '#' ? backgroundHex.substr(1) : backgroundHex;
	if (hex.size() == 6)
	{
		hex += "FF";
	}
	const unsigned long colorValue = std::stoul(hex, nullptr, 16);
	room.bgColor = sf::Color{
		static_cast<std::uint8_t>((colorValue >> 24) & 0xFF),
		static_cast<std::uint8_t>((colorValue >> 16) & 0xFF),
		static_cast<std::uint8_t>((colorValue >> 8) & 0xFF),
		static_cast<std::uint8_t>(colorValue & 0xFF)
	};
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
			if (l.contains("properties"))
			{
				for (auto& prop : l["properties"])
				{
					if (prop["name"] == "depth")
					{
						layer->depth = prop["value"].get<int>();
					}
				}
			}
			room.addObject(std::move(layer));
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
		room.addObject(std::move(player));
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
