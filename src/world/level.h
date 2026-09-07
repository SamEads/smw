#pragma once

#include <filesystem>

#include "room.h"

class Level : public Room
{
public:
    Level(const std::filesystem::path& mapPath, Game* game);
};
