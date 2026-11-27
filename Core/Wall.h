#pragma once
#include "Entity.h"

class Wall final : public Entity {
public:
    Wall(sf::Vector2i gridPosition, bool isDestructible);
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) const override;
    sf::Vector2i getGridPosition() const;
    bool isDestructible() const;

private:
    sf::Vector2i m_gridPosition;
    bool m_isDestructible;
};
