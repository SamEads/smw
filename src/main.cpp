#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <zlib.h>

#include <json.hpp>

#define GAME_WIDTH 320
#define GAME_HEIGHT 240

std::filesystem::path GetAssetDirectory(const std::filesystem::path& p)
{
	return std::filesystem::path("assets") / p;
}

class Sprite
{
private:
	struct Frame
	{
		sf::IntRect rect;
	};

	struct Animation
	{
		int from;
		int to;
	};

	std::unordered_map<std::string, Animation> animations;
	std::vector<Frame> frames;

	std::unique_ptr<sf::Texture> texture;
	std::unique_ptr<sf::Sprite> sprite;

	std::string animation;

public:
	float frame = 0.0f;

public:
	void load(const std::filesystem::path& png, const std::filesystem::path& json)
	{
		texture = std::make_unique<sf::Texture>(GetAssetDirectory(png));
		sprite = std::make_unique<sf::Sprite>(*texture.get());

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

	void draw(sf::RenderTarget& target, float x, float y)
	{
		if (!sprite || !texture) return;

		auto& animData = animations[animation];

		int frameCount = animData.to - animData.from + 1;
		float visFrame = animData.from + std::fmod(frame, frameCount);
		sprite->setTextureRect(frames[visFrame].rect);
		sprite->setPosition({ x, y });
		target.draw(*sprite.get());
	}

	void setAnimation(const std::string& anim)
	{
		if (!sprite || !texture) return;
		animation = anim;
	}
};

int main()
{
	sf::RenderWindow window(sf::VideoMode({ GAME_WIDTH, GAME_HEIGHT }), "SMW Engine 2026" );
	window.setFramerateLimit(60);
	window.setVerticalSyncEnabled(true);

	Sprite sprite;
	sprite.load("sprites/luigi_small.png", "sprites/luigi_small.json");
	sprite.setAnimation("walk");

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
		}

		sprite.frame += 0.2f;

		window.clear();
		sprite.setAnimation("walk");
		sprite.draw(window, 0, 0);
		sprite.setAnimation("jump");
		sprite.draw(window, 32, 0);
		sprite.setAnimation("fall");
		sprite.draw(window, 64, 0);
		window.display();
	}
}
