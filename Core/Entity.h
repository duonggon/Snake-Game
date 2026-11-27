#pragma once
#include <SFML/Graphics.hpp>

// Giao diện chung tối thiểu; mỗi module tự quản lý tọa độ của mình.
class Entity {
public:
    virtual ~Entity() = default;
    virtual void update(float deltaTime) = 0;
    virtual void render(sf::RenderWindow& window) const = 0;
};
