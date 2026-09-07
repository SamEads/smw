#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <array>

class Keys
{
public:
	static std::array<bool, sf::Keyboard::ScancodeCount> keys;
	static std::array<bool, sf::Keyboard::ScancodeCount> keysLast;
	static void update(bool windowFocused);
	static bool pressed(sf::Keyboard::Scancode key);
	static bool held(sf::Keyboard::Scancode key);
	static bool released(sf::Keyboard::Scancode key);
};