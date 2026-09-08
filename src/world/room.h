#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "tilemaplayer.h"
#include "collision.h"
#include "gameobject.h"
#include "game.h"

class Player;

class Room
{
public:
    explicit Room(Game* game = nullptr);

public:
	// std::vector<Collision> collisions;
    std::vector<std::unique_ptr<GameObject>> objects;

    float camX = 0, camY = 0;
    float prevCamX = 0, prevCamY = 0;
    int width = 0, height = 0;

    Player* player;

    bool stepping = false;
    int levelTime = 0;

    sf::Color bgColor;
    Game* game;
    float qblockAnimationFrame = 0.0f;

public:
    void step();
    void draw(sf::RenderTarget& target, float interp);
    std::vector<const Collision*> queryCollisions(const sf::FloatRect& area) const;
    std::vector<GameObject*> queryObjects(const sf::FloatRect& area,
        const GameObject* ignore = nullptr) const;
    std::vector<GameObject*> queryObjects(const sf::FloatRect& area,
        ObjectCategory category, const GameObject* ignore = nullptr) const;
    void addObject(std::unique_ptr<GameObject> gameObject);
    void queueFree(GameObject* gameObject);
    template<typename T, typename... Args>
    T* create(Args&&... args)
    {
        std::unique_ptr<T> obj = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = obj.get();
        addObject(std::move(obj));
        return ptr;
    }

private:
    int timerDecrementer = 0;
    float internalCamX = 0;
    float internalCamY = 0;
    std::vector<GameObject*> queuedFree;
    std::vector<std::unique_ptr<GameObject>> queuedAdd;
};