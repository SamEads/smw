#pragma once

#include <SFML/Audio/Sound.hpp>
#include <filesystem>
#include <memory>
#include <string>

class Sound
{
private:
    sf::Sound sound;
    std::string path;

public:
    Sound(const std::string& path, const sf::SoundBuffer& buffer);

public:
    static std::shared_ptr<Sound> play(const std::filesystem::path& path, float volume = 1.0f, float pitch = 1.0f);
    static void stop(const std::filesystem::path& path);
    static void stop(const std::shared_ptr<Sound>& sound);
    static bool isPlaying(const std::filesystem::path& path);
    static bool isPlaying(const std::shared_ptr<Sound>& sound);

    void stop();
};