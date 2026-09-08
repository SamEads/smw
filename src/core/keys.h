#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <array>

class Keys
{
public:
#ifdef SFML3
	static std::array<bool, sf::Keyboard::ScancodeCount> keys;
	static std::array<bool, sf::Keyboard::ScancodeCount> keysLast;
#else
	static std::array<bool, sf::Keyboard::Scancode::ScancodeCount> keys;
	static std::array<bool, sf::Keyboard::Scancode::ScancodeCount> keysLast;
#endif
	static void update(bool windowFocused);
	static bool pressed(sf::Keyboard::Scancode key);
	static bool held(sf::Keyboard::Scancode key);
	static bool released(sf::Keyboard::Scancode key);
};