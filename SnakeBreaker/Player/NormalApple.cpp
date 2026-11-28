#include "NormalApple.h"
#include "Snake.h"

// ============================================================================
// File: Player/NormalApple.cpp
// Phụ trách: Thành viên 2 – Đào Duy Khôi (Snake & Apple System)
// Mô tả: Cài đặt lớp NormalApple (Táo đỏ).
// ============================================================================

NormalApple::NormalApple(const sf::Vector2i& gridPosition)
    : Apple(gridPosition, sf::Color(235, 45, 45)) {}

void NormalApple::onEat(Snake& snake, GamePhase currentPhase) {
    if (!m_isActive) {
        return;
    }

    // 1. Tăng chiều dài rắn thêm 1 đốt (Snake::grow tự giới hạn tối đa 12 đốt)
    snake.grow(config::NORMAL_APPLE_GROWTH);

    // 2. Cộng năng lượng ở Giai đoạn 1 hoặc cộng đạn ở Giai đoạn 2
    if (currentPhase == GamePhase::SURVIVAL_PHASE) {
        snake.addEnergy(config::NORMAL_APPLE_ENERGY); // +20% năng lượng phá tường
    } else {
        snake.addAmmo(config::NORMAL_APPLE_AMMO);     // +1 viên đạn
    }

    // 3. Tăng tổng số táo đã ăn và đánh dấu quả táo đã được tiêu thụ
    snake.incrementAppleCount();
    m_isActive = false;
}

void NormalApple::render(sf::RenderWindow& window) {
    if (!m_isActive) {
        return;
    }

    // Vẽ thân quả táo đỏ
    Apple::render(window);

    // Vẽ thêm cuống lá xanh nhỏ phía trên quả táo đỏ
    sf::RectangleShape leafShape(sf::Vector2f(6.0f, 6.0f));
    const sf::Vector2f tilePixel = config::gridToPixel(m_gridPosition);
    leafShape.setPosition(
        tilePixel.x + static_cast<float>(config::TILE_SIZE) * 0.5f - 2.0f,
        tilePixel.y + 3.0f
    );
    leafShape.setFillColor(sf::Color(50, 205, 50));
    window.draw(leafShape);
}
