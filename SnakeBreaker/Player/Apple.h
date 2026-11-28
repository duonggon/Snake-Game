#pragma once
#include <string>
#include "../Core/Entity.h"
#include "../Core/GameState.h"

class Snake;

// ============================================================================
// File: Player/Apple.h
// Phụ trách: Thành viên 2 – Đào Duy Khôi (Snake & Apple System)
// Mô tả: Lớp cơ sở trừu tượng (Abstract Class) cho hệ thống 3 loại táo:
//        - NormalApple (Táo đỏ - 85%)
//        - HealthApple (Táo vàng - 5%)
//        - EnergyApple (Táo xanh dương - 10%)
//        Thể hiện rõ tính Kế thừa (kế thừa Entity), Trừu tượng và Đa hình
//        thông qua phương thức thuần ảo onEat() và render().
// ============================================================================

enum class AppleType {
    NORMAL_APPLE,
    HEALTH_APPLE,
    ENERGY_APPLE
};

class Apple : public Entity {
public:
    Apple(const sf::Vector2i& gridPosition, const sf::Color& color);
    ~Apple() override = default;

    // Phương thức thuần ảo xử lý hiệu ứng riêng khi rắn ăn từng loại táo (Polymorphism)
    virtual void onEat(Snake& snake, GamePhase currentPhase) = 0;

    // Hàm nạp chồng tiện ích cho phép truyền bool isBossPhase hoặc gọi mặc định ở Giai đoạn 1
    void onEat(Snake& snake, bool isBossPhase = false);

    // Cập nhật hiệu ứng nhịp đập nhẹ (pulsing animation) cho quả táo
    void update(float gameDeltaTime) override;

    // Vẽ quả táo lên cửa sổ SFML (các lớp con sẽ override để tùy biến màu sắc/hình dáng)
    void render(sf::RenderWindow& window) override;

    // Lấy loại táo và tên hiển thị của táo
    virtual AppleType getType() const = 0;
    virtual std::string getName() const = 0;

    const sf::Color& getColor() const { return m_color; }

protected:
    sf::Color m_color;      // Màu sắc đặc trưng của loại táo
    float m_animationTimer; // Bộ đếm thời gian phục vụ hiệu ứng co giãn nhẹ trên bản đồ
};
