#pragma once
#include "Config.h"

// ============================================================================
// File: Core/Entity.h
// Mô tả: Lớp cơ sở trừu tượng (Abstract Base Class) cho toàn bộ thực thể trong
//        game: Snake, Apple, Wall, Boss.
//        Thể hiện 4 tính chất OOP:
//        - Abstraction: Lớp trừu tượng với phương thức thuần ảo update() và render().
//        - Inheritance: Lớp cha để Snake, Apple, Wall, Boss kế thừa.
//        - Polymorphism: Các lớp con ghi đè (override) update() và render().
//        - Encapsulation: Các thuộc tính tọa độ được bảo vệ (protected) và
//          truy cập qua getter/setter chuẩn.
// ============================================================================

class Entity {
public:
    Entity()
        : m_gridPosition(0, 0)
        , m_pixelPosition(0.0f, 0.0f)
        , m_isActive(true) {}

    explicit Entity(const sf::Vector2i& gridPosition)
        : m_gridPosition(gridPosition)
        , m_pixelPosition(config::gridToPixel(gridPosition))
        , m_isActive(true) {}

    // Hủy ảo (virtual destructor) bắt buộc để giải phóng bộ nhớ đa hình an toàn
    virtual ~Entity() = default;

    // Phương thức thuần ảo cập nhật trạng thái thực thể theo gameDeltaTime
    virtual void update(float gameDeltaTime) = 0;

    // Phương thức thuần ảo vẽ thực thể lên cửa sổ SFML
    virtual void render(sf::RenderWindow& window) = 0;

    // Lấy và đặt vị trí trên lưới (gridPosition)
    const sf::Vector2i& getGridPosition() const { return m_gridPosition; }

    virtual void setGridPosition(const sf::Vector2i& gridPosition) {
        m_gridPosition = gridPosition;
        m_pixelPosition = config::gridToPixel(gridPosition);
    }

    // Lấy vị trí pixel trên màn hình (pixelPosition)
    const sf::Vector2f& getPixelPosition() const { return m_pixelPosition; }

    // Kiểm tra trạng thái hoạt động của thực thể
    bool isActive() const { return m_isActive; }
    void setActive(bool isActive) { m_isActive = isActive; }

protected:
    sf::Vector2i m_gridPosition;   // Vị trí trên lưới (ô X, ô Y)
    sf::Vector2f m_pixelPosition;  // Vị trí theo pixel tương ứng trên cửa sổ
    bool m_isActive;               // Trạng thái tồn tại/hoạt động của thực thể
};
