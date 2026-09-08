#include "sound.h"
#include "assets.h"

#include <SFML/Audio/SoundBuffer.hpp>

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <memory>

std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> buffers;
std::unordered_map<std::string, std::vector<std::shared_ptr<Sound>>> soundPools;

namespace
{
std::string getSoundKey(const std::filesystem::path& path)
{
    return GetAssetDirectory(path).lexically_normal().generic_string();
}

sf::SoundBuffer& getBuffer(const std::filesystem::path& path, const std::string& key)
{
    auto it = buffers.find(key);

    if (it != buffers.end())
        return *it->second;

    auto buffer = std::make_unique<sf::SoundBuffer>();
    buffer->loadFromFile(GetAssetDirectory(path));

    return *buffers.emplace(
        key,
        std::move(buffer)
    ).first->second;
}
}

Sound::Sound(const std::string& path, const sf::SoundBuffer& buffer) : sound(buffer), path(path)
{
}

void Sound::preload(const std::filesystem::path& soundPath, int count)
{
    const std::string key = getSoundKey(soundPath);
    const auto& buffer = getBuffer(soundPath, key);

    auto& pool = soundPools[key];

    while (static_cast<int>(pool.size()) < count)
    {
        pool.push_back(std::make_shared<Sound>(key, buffer));
    }
}

std::shared_ptr<Sound> Sound::play(const std::filesystem::path& soundPath, float volume, float pitch)
{
    const std::string key = getSoundKey(soundPath);

    auto poolIt = soundPools.find(key);

    if (poolIt == soundPools.end())
    {
        preload(soundPath, 8);
        poolIt = soundPools.find(key);
    }

    auto& pool = poolIt->second;

    for (auto& sound : pool)
    {
        if (sound->sound.getStatus() == sf::SoundSource::Status::Stopped)
        {
            sound->sound.setVolume(volume * 100.0f);
            sound->sound.setPitch(pitch);
            sound->sound.play();

            return sound;
        }
    }

    // No free voice.
    // Steal the first one instead of allocating another sf::Sound.
    auto& sound = pool.front();

    sound->sound.stop();
    sound->sound.setVolume(volume * 100.0f);
    sound->sound.setPitch(pitch);
    sound->sound.play();

    return sound;
}

void Sound::stop(const std::filesystem::path& soundPath)
{
    const std::string key = getSoundKey(soundPath);

    auto poolIt = soundPools.find(key);

    if (poolIt == soundPools.end())
        return;

    for (auto& sound : poolIt->second)
        sound->stop();
}

void Sound::stop(const std::shared_ptr<Sound>& sound)
{
    if (sound)
        sound->stop();
}

bool Sound::isPlaying(const std::filesystem::path& soundPath)
{
    const std::string key = getSoundKey(soundPath);

    auto poolIt = soundPools.find(key);

    if (poolIt == soundPools.end())
        return false;

    for (const auto& sound : poolIt->second)
    {
        if (sound->sound.getStatus() == sf::SoundSource::Status::Playing)
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