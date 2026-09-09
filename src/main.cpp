#include <SFML/Graphics.hpp>
#include <cmath>

#include "level.h"
#include "enums.h"
#include "assets.h"
#include "keys.h"
#include "font.h"
#include "game.h"
#include "core/mathhelper.h"
#include "render/textures.h"
#include <SFML/Audio/Music.hpp>
#include<iostream>
int main()
{
	// bool _soundInitResult = sf::PlaybackDevice::setDeviceToDefault();
	sf::RenderWindow window(sf::VideoMode({ 640, 480 }), "SUPER FUCKING MARIO WORLD!!!!!!!!!!!!! TRANSGENDER");
	window.setVerticalSyncEnabled(true);

	sf::RenderTexture t({ GAME_WIDTH, GAME_HEIGHT });

	Game game;
	game.timer.setTickRate(60);
	Level level(GetAssetDirectory("levels/level1.tmj"), &game);

	Font::SMALL.initialize("sprites/hud/small_font.png", 8, 8,
	{
		{ 'A', 0 },  { 'B',  1 }, { 'C',  2 },
		{ 'D', 3 },  { 'E',  4 }, { 'F',  5 },
		{ 'G', 6 },  { 'H',  7 }, { 'I',  8 },
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
		{ '\'', 42 },{ '\"',43 },{ 'x', 44 },
	});
	Font::POINTS.initialize("sprites/hud/points_font.png", 8, 16,
	{
		{ '0', 0 }, { '1', 1 }, { '2', 2 },
		{ '3', 3 }, { '4', 4 }, { '5', 5 },
		{ '6', 6 }, { '7', 7 }, { '8', 8 },
		{ '9', 9 },
	});
	
	/*
	float scrollValueX = 0;
	float scrollValueY = 0;
	while (window.isOpen())
	{
		Keys::update(window.hasFocus());
		while (const std::optional event = window.pollEvent())
		{
			if (const auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>())
			{
				float delta = scroll->delta;

				if (Keys::held(sf::Keyboard::Scancode::LShift))
					scrollValueX += delta * 2.0f;
				else
					scrollValueY += delta * 2.0f;
			}
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
			else if (const auto* resized = event->getIf<sf::Event::Resized>())
			{
				window.setView(sf::View(
					sf::FloatRect(
						{ 0.f, 0.f },
						{ static_cast<float>(resized->size.x),
						static_cast<float>(resized->size.y) }
					)
				));
			}
		}
		window.clear();
		sf::Color topGradient(0, 0, 96);
		sf::Color bottomGradient(0, 0, 255);
		sf::VertexArray background(sf::PrimitiveType::Triangles, 6);

		for (int i = 0; i < 6; ++i)
		{
			background[0].position = sf::Vector2f(0, 0);
			background[1].position = sf::Vector2f(window.getSize().x, 0);
			background[2].position = sf::Vector2f(window.getSize().x, window.getSize().y);
			background[3].position = sf::Vector2f(window.getSize().x, window.getSize().y);
			background[4].position = sf::Vector2f(0, window.getSize().y);
			background[5].position = sf::Vector2f(0, 0);

			float factor = background[i].position.y / window.getSize().y;

			background[i].color = sf::Color(
				static_cast<uint8_t>((1.0f - factor) * topGradient.r + factor * bottomGradient.r),
				static_cast<uint8_t>((1.0f - factor) * topGradient.g + factor * bottomGradient.g),
				static_cast<uint8_t>((1.0f - factor) * topGradient.b + factor * bottomGradient.b)
			);
		}
		sf::Sprite durr(Textures::get("tiles/ground.png"));
		durr.setScale({ 2, 2 });
		durr.setPosition({ scrollValueX * 32, scrollValueY * 32 });
		window.draw(background);
		window.draw(durr);
		Font::SMALL.draw("TILESET NO. 0", window, 0, 0);

		sf::RectangleShape red({ 32, 32 });
		red.setFillColor({ 255, 0, 0, 255 });
		auto mousePos = sf::Mouse::getPosition(window);
		mousePos.x = floorf(mousePos.x / 32.0f) * 32.0f;
		mousePos.y = floorf(mousePos.y / 32.0f) * 32.0f;
		red.setPosition({ mousePos.x, mousePos.y });
		sf::RenderStates states;
		states.blendMode = sf::BlendAdd;

		window.draw(red, states);

		window.display();
	}
	*/

	// sf::Music music("assets/music/baci_perugina.ogg");
	// music.setVolume(90.0f);
	// music.setPitch(1.1f);
	// music.setLooping(true);
	// music.play();
	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();
		}

		game.timer.update();
        
        const int ticks = game.timer.getTickCount();
		for (int i = 0; i < ticks; ++i)
		{
			Keys::update(window.hasFocus());
			level.step();
		}

        float interp = game.timer.getAlpha();
		if (interp < 0.0f || interp > 1.0f) std::cout << interp << "\n";

		t.clear();
		level.draw(t, interp);
		t.display();

		const auto windowSize = window.getSize();
        sf::View view(sf::FloatRect{ { 0, 0 }, { (float)windowSize.x, (float)windowSize.y } });
        view.setCenter({ windowSize.x / 2.0f, windowSize.y / 2.0f });
        window.setView(view);

		window.clear();
		sf::Sprite bg(Textures::get("bg/map.png"));
		sf::Sprite ss(t.getTexture());
		auto rtSize = t.getSize();
		float gameScaleX = windowSize.x / (float)GAME_WIDTH;
		float gameScaleY = windowSize.y / (float)GAME_HEIGHT;
		float gameScaleMin = std::min(gameScaleX, gameScaleY);
		float flooredGameScaleMin = std::floorf(gameScaleMin);
		if (flooredGameScaleMin != 0.0f)
			gameScaleMin = flooredGameScaleMin;
		float myRelationX =  rtSize.x / (float)GAME_WIDTH;
		float myRelationY = rtSize.y / (float)GAME_HEIGHT;
		bg.setScale({ gameScaleMin, gameScaleMin });
		auto bgSize = bg.getTexture().getSize();
		float repeatX = MathHelper::max(1.0f, std::ceilf(windowSize.x / (gameScaleMin * (float)bgSize.x)));
		float repeatY = MathHelper::max(1.0f, std::ceilf(windowSize.y / (gameScaleMin * (float)bgSize.y)));
		for (int i = 0; i < repeatX; ++i)
		{
			for (int j = 0; j < repeatY; ++j)
			{
				bg.setPosition({ i * bgSize.x * gameScaleMin, j * bgSize.y * gameScaleMin });
				window.draw(bg);
			}
		}
		ss.setScale({ gameScaleMin / myRelationX, gameScaleMin / myRelationY });
		ss.setOrigin({ (float)rtSize.x / 2.0f, (float)rtSize.y / 2.0f });
		ss.setPosition({ std::floorf(windowSize.x / 2.0f), std::floorf(windowSize.y / 2.0f) });
		float borderSize = std::floorf(gameScaleMin) * 2.0f;
		sf::RectangleShape rs({ rtSize.x * ss.getScale().x + borderSize, rtSize.y * ss.getScale().y + borderSize });
		rs.setFillColor({ sf::Color::Black });
		rs.setPosition({ ss.getPosition().x - rs.getSize().x / 2.0f, ss.getPosition().y - rs.getSize().y / 2.0f });
		sf::Color downColor(168, 152, 120);
		float dropShadowSize = std::floorf(gameScaleMin) * 8.0f;
		for (int i = 0; i < 3; ++i)
		{
			float dropShadowInner = std::floorf(gameScaleMin) * i * 2.0f;
			sf::RectangleShape dropShadow({ rtSize.x * ss.getScale().x - dropShadowInner, rtSize.y * ss.getScale().y - dropShadowInner });
			dropShadow.setFillColor({ downColor });
			dropShadow.setPosition({ ss.getPosition().x - dropShadow.getSize().x / 2.0f + dropShadowSize, ss.getPosition().y - dropShadow.getSize().y / 2.0f + dropShadowSize });
			window.draw(dropShadow);
			downColor.r -= 24;
			downColor.g -= 24;
			downColor.b -= 24;
		}
		window.draw(rs);
		window.draw(ss);
		window.display();
	}
}
