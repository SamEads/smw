#include <SFML/Graphics.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>

#include "level.h"
#include "enums.h"
#include "assets.h"
#include "keys.h"
#include "font.h"
#include "game.h"

int main()
{
	bool _soundInitResult = sf::PlaybackDevice::setDeviceToDefault();
	sf::RenderWindow window(sf::VideoMode({ GAME_WIDTH * 3, GAME_HEIGHT * 3 }), "SUPER FUCKING MARIO WORLD!!!!!!!!!!!!! TRANSGENDER" );
	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	sf::RenderTexture t({ GAME_WIDTH, GAME_HEIGHT });

	Game game;
	Level level(GetAssetDirectory("levels/level0.tmj"), &game);

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

		level.step();

		t.clear();
		level.draw(t);
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
