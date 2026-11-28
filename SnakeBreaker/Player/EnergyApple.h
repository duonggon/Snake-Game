#pragma once
#include "Apple.h"

// ============================================================================
// File: Player/EnergyApple.h
// Phụ trách: Thành viên 2 – Đào Duy Khôi (Snake & Apple System)
// Mô tả: EnergyApple – Táo xanh dương (Tỷ lệ xuất hiện: 10%)
//        - Tăng 60% năng lượng phá tường trong giai đoạn 1.
//        - Tăng 3 viên đạn trong giai đoạn 2.
// ============================================================================

class EnergyApple : public Apple {
public:
    explicit EnergyApple(const sf::Vector2i& gridPosition);
    ~EnergyApple() override = default;

    // Ghi đè (override) hiệu ứng khi rắn ăn Táo xanh dương
    void onEat(Snake& snake, GamePhase currentPhase) override;

    // Ghi đè (override) cách vẽ Táo xanh dương
    void render(sf::RenderWindow& window) override;

    AppleType getType() const override { return AppleType::ENERGY_APPLE; }
    std::string getName() const override { return "EnergyApple (Tao Xanh Duong)"; }
};
