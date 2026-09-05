#include <cmath>
#include "mathhelper.h"

float MathHelper::clamp(float v, float min, float max)
{
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

float MathHelper::min(float a, float b)
{
    return (a < b) ? a : b;
}

float MathHelper::max(float a, float b)
{
    return (a > b) ? a : b;
}

float MathHelper::moveToward(float from, float to, float delta)
{
    if (from < to)
        return std::min(from + delta, to);
    else
        return std::max(from - delta, to);
}
