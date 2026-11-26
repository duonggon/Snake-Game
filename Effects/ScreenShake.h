#pragma once
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
#include <random>

// =============================================================================
// LỚP QUẢN LÝ RUNG MÀN HÌNH (Effects/ScreenShake.h)
// Phụ trách: Huỳnh Nhựt Cường
//
// Cơ chế hoạt động theo thiết kế đã chốt:
// 1. Rung nhẹ nhàng, êm mắt (thang điểm từ 0 đến 10, trần tối đa khóa ở 6/10).
// 2. Lần kích hoạt đầu tiên có thời gian u1 = 1.5 giây.
// 3. Nếu kích hoạt dồn thêm khi màn hình vẫn đang rung (stackCount >= 2):
//    - Cường độ cộng dồn tăng dần nhưng KHÔNG VƯỢT QUÁ MỨC 6/10.
//    - Thời gian cộng thêm tuân theo cấp số nhân: deltaT_k = 1.5 * (0.30)^(k - 1)
//      => Tổng thời gian chuỗi rung tích lũy bằng tổng cấp số nhân hữu hạn.
// 4. Tách riêng View rung (để vẽ bản đồ) và View tĩnh (để lấy tọa độ chuột chuẩn).
// =============================================================================
class ScreenShake {
public:
    ScreenShake();

    // Kích hoạt một chấn động mới (cộng dồn nếu đang trong chuỗi rung)
    void trigger();

    // Đặt lại ngay lập tức trạng thái rung về 0 (dùng khi Restart màn chơi)
    void reset();

    // Cập nhật độ lệch camera theo thời gian thực (unscaledDeltaTime)
    void update(float unscaledDeltaTime);

    // Tạo bản sao sf::View đã cộng độ lệch rung để vẽ thế giới game
    sf::View applyToView(const sf::View& baseView) const;

    // Bật hoặc tắt nhanh hiệu ứng rung màn hình lúc đang chạy
    void setEnabled(bool enabled);
    bool isEnabled() const;

    // --- Các hàm truy vấn phục vụ kiểm thử (Unit Tests) & hiển thị thông số ---
    bool isActive() const;
    int getStackCount() const;
    float getCurrentLevel() const;              // Giá trị từ 0.0 -> tối đa 6.0 (mức 6/10)
    float getRemainingTime() const;             // Thời gian rung còn lại (giây)
    float getTotalAccumulatedDuration() const;  // Tổng cấp số nhân thời gian của đợt rung hiện tại
    sf::Vector2f getCurrentOffset() const;

private:
    bool m_enabled;
    int m_stackCount;
    float m_currentLevel;
    float m_remainingTime;
    float m_totalAccumulatedDuration;
    float m_elapsedWaveTime;
    sf::Vector2f m_currentOffset;
    std::mt19937 m_rng;
};
