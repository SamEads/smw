#pragma once

#include <array>

#include "timer.h"
#include "playercharacter.h"

enum class PowerupState
{
    SMALL,
    BIG
};

class Game
{
public:
    struct CharacterData
    {
        int score = 0;
        int tapeScore = 0;
        int lives = 5;
        int coins = 0;
        PowerupState powerup = PowerupState::SMALL;
    };

    PlayerCharacter playerCharacter = PlayerCharacter::MARIO;
    std::array<CharacterData, 2> characterData;

    Timer timer;

    CharacterData& dataFor(PlayerCharacter character)
    {
        return characterData[character == PlayerCharacter::MARIO ? 0 : 1];
    }

    const CharacterData& dataFor(PlayerCharacter character) const
    {
        return characterData[character == PlayerCharacter::MARIO ? 0 : 1];
    }
};
