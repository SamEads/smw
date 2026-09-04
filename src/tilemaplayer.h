#pragma once

#include <vector>

class TilemapLayerChunk
{
public:
    int x, y;
    int width, height;
    std::vector<int> values;
};

class TilemapLayer
{
public:
    int width, height;
    std::vector<TilemapLayerChunk> chunks;
};