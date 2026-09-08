#include "keys.h"

#ifdef SFML3
std::array<bool, sf::Keyboard::ScancodeCount> Keys::keys;
std::array<bool, sf::Keyboard::ScancodeCount> Keys::keysLast;
#else
std::array<bool, sf::Keyboard::Scancode::ScancodeCount> Keys::keys;
std::array<bool, sf::Keyboard::Scancode::ScancodeCount> Keys::keysLast;
#endif

void Keys::update(bool windowFocused)
{
	for (int i = 0; i < keys.size(); ++i)
		keysLast[i] = keys[i];

	if (!windowFocused)
    {
		for (int i = 0; i < keys.size(); ++i)
			keys[i] = false;
	}
	else for (int i = 0; i < keys.size(); ++i)
    {
		keys[i] = sf::Keyboard::isKeyPressed((sf::Keyboard::Scancode)i);
	}
}

bool Keys::pressed(sf::Keyboard::Scancode key) { return keys[(int)key] && !keysLast[(int)key]; }
bool Keys::held(sf::Keyboard::Scancode key) { return keys[(int)key]; }
bool Keys::released(sf::Keyboard::Scancode key) { return keysLast[(int)key] && !keys[(int)key]; }