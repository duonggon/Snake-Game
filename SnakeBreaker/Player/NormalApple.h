#pragma once
#include "Apple.h"

// ============================================================================
// File: Player/NormalApple.h
// Phụ trách: Thành viên 2 – Đào Duy Khôi (Snake & Apple System)
// Mô tả: NormalApple – Táo đỏ (Tỷ lệ xuất hiện: 85%)
//        - Tăng chiều dài rắn thêm 1 đốt (tối đa 12 đốt).
//        - Tăng 20% năng lượng phá tường trong giai đoạn 1.
//        - Tăng 1 viên đạn trong giai đoạn 2.
// ============================================================================

class NormalApple : public Apple {
public:
    explicit NormalApple(const sf::Vector2i& gridPosition);
    ~NormalApple() override = default;

    // Ghi đè (override) hiệu ứng khi rắn ăn Táo đỏ
    void onEat(Snake& snake, GamePhase currentPhase) override;

    // Ghi đè (override) cách vẽ Táo đỏ
    void render(sf::RenderWindow& window) override;

    AppleType getType() const override { return AppleType::NORMAL_APPLE; }
    std::string getName() const override { return "NormalApple (Tao Do)"; }
};
