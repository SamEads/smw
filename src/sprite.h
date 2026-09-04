#pragma once

#include <SFML/Graphics.hpp>

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

	std::unique_ptr<sf::Sprite> sprite;

	std::string animation;
	sf::Vector2f origin;

public:
	float frame = 0.0f;

public:
	void load(const std::filesystem::path& png, const std::filesystem::path& json);
	void draw(sf::RenderTarget& target, float x, float y);
	void setAnimation(const std::string& anim);
	void setOrigin(float x, float y);
};