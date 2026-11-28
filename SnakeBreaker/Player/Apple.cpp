#include "Apple.h"
#include <cmath>

// ============================================================================
// File: Player/Apple.cpp
// Phụ trách: Thành viên 2 – Đào Duy Khôi (Snake & Apple System)
// Mô tả: Cài đặt các phương thức dùng chung của lớp cơ sở trừu tượng Apple.
// ============================================================================

Apple::Apple(const sf::Vector2i& gridPosition, const sf::Color& color)
    : Entity(gridPosition)
    , m_color(color)
    , m_animationTimer(0.0f) {}

void Apple::onEat(Snake& snake, bool isBossPhase) {
    onEat(snake, isBossPhase ? GamePhase::BOSS_PHASE : GamePhase::SURVIVAL_PHASE);
}

void Apple::update(float gameDeltaTime) {
    m_animationTimer += gameDeltaTime;
}

void Apple::render(sf::RenderWindow& window) {
    if (!m_isActive) {
        return;
    }

    // Hiệu ứng co giãn bán kính nhẹ theo hàm sin giúp vật phẩm sinh động
    const float baseRadius = static_cast<float>(config::TILE_SIZE) * 0.38f;
    const float pulseOffset = std::sin(m_animationTimer * 5.0f) * 1.5f;
    const float radius = baseRadius + pulseOffset;

    sf::CircleShape appleShape(radius);
    const sf::Vector2f tilePixel = config::gridToPixel(m_gridPosition);
    const float centerOffset = (static_cast<float>(config::TILE_SIZE) * 0.5f) - radius;

    appleShape.setPosition(tilePixel.x + centerOffset, tilePixel.y + centerOffset);
    appleShape.setFillColor(m_color);
    appleShape.setOutlineThickness(2.0f);
    appleShape.setOutlineColor(sf::Color::White);

    window.draw(appleShape);
}
