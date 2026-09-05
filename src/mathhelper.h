#pragma once

class MathHelper
{
public:
    static float clamp(float v, float min, float max);
    static float min(float a, float b);
    static float max(float a, float b);
    static float moveToward(float from, float to, float delta);
};