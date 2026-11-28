#pragma once
#include "Apple.h"

// ============================================================================
// File: Player/HealthApple.h
// Phụ trách: Thành viên 2 – Đào Duy Khôi (Snake & Apple System)
// Mô tả: HealthApple – Táo vàng (Tỷ lệ xuất hiện: 5%)
//        - Hồi 1 mạng cho rắn.
//        - Không vượt quá 3 mạng (MAX_LIVES).
// ============================================================================

class HealthApple : public Apple {
public:
    explicit HealthApple(const sf::Vector2i& gridPosition);
    ~HealthApple() override = default;

    // Ghi đè (override) hiệu ứng khi rắn ăn Táo vàng
    void onEat(Snake& snake, GamePhase currentPhase) override;

    // Ghi đè (override) cách vẽ Táo vàng
    void render(sf::RenderWindow& window) override;

    AppleType getType() const override { return AppleType::HEALTH_APPLE; }
    std::string getName() const override { return "HealthApple (Tao Vang)"; }
};
