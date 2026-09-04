#pragma once

#include "sprite.h"

class Player
{
public:
    float x, y;
    float vspd, hspd;
    float grav;

	Sprite sprite;
};