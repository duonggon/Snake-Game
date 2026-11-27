#include "Wall.h"
#include "Config.h"

Wall::Wall(sf::Vector2i gridPosition, bool isDestructible)
    : m_gridPosition(gridPosition), m_isDestructible(isDestructible) {}

void Wall::update(float) {
    // Tường đứng yên, không cần cập nhật chuyển động.
}

void Wall::render(sf::RenderWindow& window) const {
    sf::RectangleShape shape({float(config::TILE_SIZE - 2), float(config::TILE_SIZE - 2)});
    shape.setPosition({float(m_gridPosition.x * config::TILE_SIZE + 1),
                       float(m_gridPosition.y * config::TILE_SIZE + 1)});
    shape.setFillColor(m_isDestructible ? sf::Color(157, 100, 61) : sf::Color(78, 91, 110));
    window.draw(shape);
}

sf::Vector2i Wall::getGridPosition() const { return m_gridPosition; }
bool Wall::isDestructible() const { return m_isDestructible; }
