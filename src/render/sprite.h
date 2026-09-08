#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

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

	struct Definition
	{
		std::unordered_map<std::string, Animation> animations;
		std::vector<Frame> frames;
		float width = 0.0f;
		float height = 0.0f;
		bool hasAnimations = false;
	};

	std::shared_ptr<const Definition> definition;

	std::unique_ptr<sf::Sprite> sprite;

	std::string animation;
	sf::Vector2f origin;

public:
	float frame = 0.0f;
	bool flipX = false;
	bool flipY = false;
	float width, height;

public:
	void load(const std::filesystem::path& png, const std::filesystem::path& json);
	void draw(sf::RenderTarget& target, float x, float y);
	void play(const std::string& anim);
	void setOrigin(float x, float y);
	int getFrameCount();
};