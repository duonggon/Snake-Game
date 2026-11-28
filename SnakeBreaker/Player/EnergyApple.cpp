#include "EnergyApple.h"
#include "Snake.h"

// ============================================================================
// File: Player/EnergyApple.cpp
// Phụ trách: Thành viên 2 – Đào Duy Khôi (Snake & Apple System)
// Mô tả: Cài đặt lớp EnergyApple (Táo xanh dương).
// ============================================================================

EnergyApple::EnergyApple(const sf::Vector2i& gridPosition)
    : Apple(gridPosition, sf::Color(30, 144, 255)) {}

void EnergyApple::onEat(Snake& snake, GamePhase currentPhase) {
    if (!m_isActive) {
        return;
    }

    // 1. Giai đoạn 1: +60% năng lượng phá tường | Giai đoạn 2: +3 viên đạn
    if (currentPhase == GamePhase::SURVIVAL_PHASE) {
        snake.addEnergy(config::ENERGY_APPLE_ENERGY);
    } else {
        snake.addAmmo(config::ENERGY_APPLE_AMMO);
    }

    // 2. Tăng tổng số táo đã ăn (tính tất cả các loại táo)
    snake.incrementAppleCount();
    m_isActive = false;
}

void EnergyApple::render(sf::RenderWindow& window) {
    if (!m_isActive) {
        return;
    }

    // Vẽ thân quả táo xanh dương
    Apple::render(window);

    // Vẽ lõi năng lượng màu Cyan sáng ở giữa quả táo xanh dương
    const sf::Vector2f tilePixel = config::gridToPixel(m_gridPosition);
    const float coreRadius = 5.0f;
    sf::CircleShape energyCore(coreRadius);
    energyCore.setPosition(
        tilePixel.x + static_cast<float>(config::TILE_SIZE) * 0.5f - coreRadius,
        tilePixel.y + static_cast<float>(config::TILE_SIZE) * 0.5f - coreRadius
    );
    energyCore.setFillColor(sf::Color::Cyan);
    window.draw(energyCore);
}
