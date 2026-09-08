#include "sound.h"
#include "assets.h"

#include <SFML/Audio/SoundBuffer.hpp>
#include <algorithm>
#include <unordered_map>
#include <vector>

namespace
{
std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> buffers;
std::vector<std::shared_ptr<Sound>> activeSounds;

std::string getSoundKey(const std::filesystem::path& path)
{
    return GetAssetDirectory(path).lexically_normal().generic_string();
}

sf::SoundBuffer& getBuffer(const std::filesystem::path& path, const std::string& key)
{
    auto buffer = buffers.find(key);
    if (buffer == buffers.end())
    {
        auto loadedBuffer = std::make_unique<sf::SoundBuffer>(GetAssetDirectory(path));
        buffer = buffers.emplace(key, std::move(loadedBuffer)).first;
    }
    return *buffer->second;
}
}

Sound::Sound(const std::string& path, const sf::SoundBuffer& buffer) : sound(buffer), path(path)
{
}

#include <iostream>
#include "../core/profiler.h"
std::shared_ptr<Sound> Sound::play(const std::filesystem::path& soundPath, float volume, float pitch)
{
    activeSounds.erase(
        std::remove_if(activeSounds.begin(), activeSounds.end(), [](const std::shared_ptr<Sound>& sound)
        {
            return sound->sound.getStatus() == sf::SoundSource::Status::Stopped;
        }),
        activeSounds.end());

    std::string key = getSoundKey(soundPath);

    auto sound = std::make_shared<Sound>(key, getBuffer(soundPath, key));
    sound->sound.setVolume(volume * 100.0f);
    sound->sound.setPitch(pitch);
    sound->sound.play();
    activeSounds.push_back(sound);
    return sound;
}

void Sound::stop(const std::filesystem::path& soundPath)
{
    const std::string key = getSoundKey(soundPath);
    for (const auto& sound : activeSounds)
    {
        if (sound->path == key)
        {
            sound->stop();
        }
    }
}

void Sound::stop(const std::shared_ptr<Sound>& sound)
{
    if (sound)
    {
        sound->stop();
    }
}

bool Sound::isPlaying(const std::filesystem::path& soundPath)
{
    const std::string key = getSoundKey(soundPath);
    for (const auto& sound : activeSounds)
    {
        if (sound->path == key && sound->sound.getStatus() == sf::SoundSource::Status::Playing)
        {
            return true;
        }
    }
    return false;
}

bool Sound::isPlaying(const std::shared_ptr<Sound>& sound)
{
    return sound && sound->sound.getStatus() == sf::SoundSource::Status::Playing;
}

void Sound::stop()
{
    sound.stop();
}