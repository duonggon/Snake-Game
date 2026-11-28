#include "HealthApple.h"
#include "Snake.h"

// ============================================================================
// File: Player/HealthApple.cpp
// Phụ trách: Thành viên 2 – Đào Duy Khôi (Snake & Apple System)
// Mô tả: Cài đặt lớp HealthApple (Táo vàng).
// ============================================================================

HealthApple::HealthApple(const sf::Vector2i& gridPosition)
    : Apple(gridPosition, sf::Color(255, 215, 0)) {}

void HealthApple::onEat(Snake& snake, GamePhase /*currentPhase*/) {
    if (!m_isActive) {
        return;
    }

    // 1. Hồi 1 mạng cho rắn (Snake::addLife tự động giới hạn không vượt quá 3 mạng)
    snake.addLife(config::HEALTH_APPLE_LIFE);

    // 2. Tăng tổng số táo đã ăn (điều kiện chuyển giai đoạn tính tất cả các loại táo)
    snake.incrementAppleCount();
    m_isActive = false;
}

void HealthApple::render(sf::RenderWindow& window) {
    if (!m_isActive) {
        return;
    }

    // Vẽ thân quả táo vàng
    Apple::render(window);

    // Vẽ biểu tượng dấu cộng (+) màu trắng ở tâm quả táo vàng để biểu thị hồi mạng
    const sf::Vector2f tilePixel = config::gridToPixel(m_gridPosition);
    const float centerX = tilePixel.x + static_cast<float>(config::TILE_SIZE) * 0.5f;
    const float centerY = tilePixel.y + static_cast<float>(config::TILE_SIZE) * 0.5f;

    sf::RectangleShape horizontalBar(sf::Vector2f(12.0f, 4.0f));
    horizontalBar.setPosition(centerX - 6.0f, centerY - 2.0f);
    horizontalBar.setFillColor(sf::Color::White);

    sf::RectangleShape verticalBar(sf::Vector2f(4.0f, 12.0f));
    verticalBar.setPosition(centerX - 2.0f, centerY - 6.0f);
    verticalBar.setFillColor(sf::Color::White);

    window.draw(horizontalBar);
    window.draw(verticalBar);
}
