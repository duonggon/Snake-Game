#pragma once
#include "SfmlCompat.h"

// ============================================================================
// File: Core/Config.h
// Mô tả: Lưu trữ toàn bộ các hằng số cân bằng game và cấu hình hệ thống.
//        Tuân thủ quy tắc: Namespace viết thường (config), hằng số viết hoa
//        ngăn cách bằng dấu gạch dưới (UPPER_SNAKE_CASE).
// ============================================================================

namespace config {

    // ------------------------------------------------------------------------
    // 1. CẤU HÌNH BẢN ĐỒ & CỬA SỔ (Đã thống nhất trong đề bài)
    // ------------------------------------------------------------------------
    constexpr int MAP_WIDTH = 24;               // Chiều rộng bản đồ: 24 ô
    constexpr int MAP_HEIGHT = 18;              // Chiều cao bản đồ: 18 ô
    constexpr int TILE_SIZE = 40;               // Kích thước mỗi ô: 40x40 pixel

    constexpr int WINDOW_WIDTH = 960;           // 24 * 40 = 960 pixel
    constexpr int WINDOW_HEIGHT = 800;          // 18 * 40 + 80 (HUD) = 800 pixel
    constexpr int HUD_HEIGHT = 80;              // [ĐỀ XUẤT] Chiều cao vùng HUD phía trên (800 - 720 = 80 pixel)
    constexpr int MAP_OFFSET_Y = HUD_HEIGHT;    // [ĐỀ XUẤT] Độ lệch trục Y để vẽ bản đồ ngay bên dưới HUD

    // ------------------------------------------------------------------------
    // 2. THÔNG SỐ CỦA RẮN - THÀNH VIÊN 2 (Đã thống nhất trong đề bài)
    // ------------------------------------------------------------------------
    constexpr int INITIAL_SNAKE_LENGTH = 3;     // Chiều dài ban đầu: 3 đốt
    constexpr int MAX_SNAKE_LENGTH = 12;        // Chiều dài tối đa: 12 đốt
    constexpr int INITIAL_LIVES = 3;            // Số mạng ban đầu: 3 mạng
    constexpr int MAX_LIVES = 3;                // Số mạng tối đa: 3 mạng
    constexpr float INVINCIBLE_DURATION = 1.5f; // Thời gian bất tử sau khi mất mạng: 1,5 giây

    // Các thông số chuyển động & hiển thị của rắn ([ĐỀ XUẤT] cân bằng gameplay)
    constexpr float DEFAULT_SNAKE_SPEED = 6.0f; // [ĐỀ XUẤT] Tốc độ di chuyển mặc định của rắn: 6 ô/giây
    constexpr float BLINK_INTERVAL = 0.12f;     // [ĐỀ XUẤT] Chu kỳ nhấp nháy khi bất tử: 0,12 giây/lần
    constexpr int SAFE_SPAWN_X = 5;             // [ĐỀ XUẤT] Tọa độ X mặc định khi khởi tạo/hồi sinh rắn
    constexpr int SAFE_SPAWN_Y = 9;             // [ĐỀ XUẤT] Tọa độ Y mặc định khi khởi tạo/hồi sinh rắn

    // ------------------------------------------------------------------------
    // 3. NĂNG LƯỢNG PHÁ TƯỜNG & ĐẠN - THÀNH VIÊN 2 & 4 (Đã thống nhất)
    // ------------------------------------------------------------------------
    constexpr int MAX_ENERGY = 100;             // Năng lượng tối đa để kích hoạt phá tường: 100%
    constexpr int INITIAL_ENERGY = 0;           // Năng lượng khởi đầu giai đoạn 1: 0%

    constexpr int MAX_AMMO = 5;                 // Sức chứa đạn tối đa trong giai đoạn 2: 5 viên
    constexpr int INITIAL_BOSS_PHASE_AMMO = 3;  // Số đạn cấp ban đầu khi bước vào giai đoạn đánh Boss: 3 viên

    // ------------------------------------------------------------------------
    // 4. HỆ THỐNG TÁO - THÀNH VIÊN 2 (Đã thống nhất trong đề bài)
    // ------------------------------------------------------------------------
    // Tỷ lệ xuất hiện của 3 loại táo (Tổng = 100%)
    constexpr int NORMAL_APPLE_RATE = 85;       // Táo đỏ (NormalApple): 85%
    constexpr int ENERGY_APPLE_RATE = 10;       // Táo xanh dương (EnergyApple): 10%
    constexpr int HEALTH_APPLE_RATE = 5;        // Táo vàng (HealthApple): 5%

    // Giá trị hồi phục / cộng thêm của từng loại táo
    constexpr int NORMAL_APPLE_GROWTH = 1;      // Táo đỏ tăng 1 đốt chiều dài
    constexpr int NORMAL_APPLE_ENERGY = 20;     // Táo đỏ tăng 20% năng lượng ở giai đoạn 1
    constexpr int NORMAL_APPLE_AMMO = 1;        // Táo đỏ tăng 1 viên đạn ở giai đoạn 2

    constexpr int HEALTH_APPLE_LIFE = 1;        // Táo vàng hồi 1 mạng (tối đa 3 mạng)

    constexpr int ENERGY_APPLE_ENERGY = 60;     // Táo xanh dương tăng 60% năng lượng ở giai đoạn 1
    constexpr int ENERGY_APPLE_AMMO = 3;        // Táo xanh dương tăng 3 viên đạn ở giai đoạn 2

    // Điều kiện chuyển từ Giai đoạn 1 sang Giai đoạn 2
    constexpr int APPLES_TO_BOSS_PHASE = 10;    // Ăn đủ 10 quả táo (tính tất cả các loại)

    // ------------------------------------------------------------------------
    // 5. THÔNG SỐ TƯỜNG & BOSS (Dùng để tham chiếu giao tiếp liên module)
    // ------------------------------------------------------------------------
    constexpr int MAX_INNER_WALLS = 30;         // Giới hạn tối đa 30 ô tường bên trong
    constexpr int BOSS_SIZE_TILES = 2;          // Kích thước Boss: 2x2 ô
    constexpr int BOSS_MAX_HEALTH = 100;        // Máu tối đa của Boss: 100 HP
    constexpr int BULLET_DAMAGE = 15;           // Mỗi phát bắn trúng gây 15 HP sát thương

    // ------------------------------------------------------------------------
    // 6. HÀM TIỆN ÍCH CHUYỂN ĐỔI TỌA ĐỘ LƯỚI <-> PIXEL DÙNG CHUNG
    // ------------------------------------------------------------------------
    // Chuyển tọa độ ô lưới (gridPosition) sang tọa độ pixel trên cửa sổ (pixelPosition)
    inline sf::Vector2f gridToPixel(const sf::Vector2i& gridPosition, bool includeHudOffset = true) {
        float offsetY = includeHudOffset ? static_cast<float>(MAP_OFFSET_Y) : 0.0f;
        return sf::Vector2f(
            static_cast<float>(gridPosition.x * TILE_SIZE),
            static_cast<float>(gridPosition.y * TILE_SIZE) + offsetY
        );
    }

    // Kiểm tra một tọa độ lưới có nằm trong vùng bên trong bản đồ (trừ tường viền ngoài) hay không
    inline bool isInsideInnerMap(const sf::Vector2i& gridPosition) {
        return gridPosition.x >= 1 && gridPosition.x < (MAP_WIDTH - 1) &&
               gridPosition.y >= 1 && gridPosition.y < (MAP_HEIGHT - 1);
    }

    // Kiểm tra một tọa độ lưới có chạm vào tường viền ngoài cố định hay không
    inline bool isBorderWall(const sf::Vector2i& gridPosition) {
        return gridPosition.x <= 0 || gridPosition.x >= (MAP_WIDTH - 1) ||
               gridPosition.y <= 0 || gridPosition.y >= (MAP_HEIGHT - 1);
    }

} // namespace config
