#pragma once

#include <array>
#include <cstddef>
#include <random>
#include <type_traits>

class MathHelper
{
private:
    static std::mt19937& randomEngine();

public:
    static float clamp(float v, float min, float max);
    static float min(float a, float b);
    static float max(float a, float b);
    static float moveToward(float from, float to, float delta);
    static int randomRange(int min, int max);
    static float randomFloat(float min, float max);

    template <typename T, typename... Ts>
    static typename std::common_type<T, Ts...>::type choose(T first, Ts... rest)
    {
        typedef typename std::common_type<T, Ts...>::type Choice;
        const std::array<Choice, sizeof...(Ts) + 1> values = {{ static_cast<Choice>(first), static_cast<Choice>(rest)... }};
        std::uniform_int_distribution<std::size_t> distribution(0, values.size() - 1);
        return values[distribution(randomEngine())];
    }
};