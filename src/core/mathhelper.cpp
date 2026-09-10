#include <cmath>
#include <random>
#include "mathhelper.h"

std::mt19937& MathHelper::randomEngine()
{
    static std::mt19937 engine(std::random_device{}());
    return engine;
}

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

int MathHelper::randomRange(int min, int max)
{
    if (min > max)
        std::swap(min, max);

    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(randomEngine());
}

float MathHelper::randomFloat(float min, float max)
{
    if (min > max)
        std::swap(min, max);

    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(randomEngine());
}

float MathHelper::lerp(float a, float b, float alpha)
{
    if (alpha >= 1.0f) return a;
    if (alpha <= 0) return b;
    return (1.0f - alpha) * a + alpha * b;
}

