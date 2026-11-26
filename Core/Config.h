#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

// =============================================================================
// TỆP CẤU HÌNH THÔNG SỐ CHUNG CỦA GAME (Core/Config.h)
// Tuân thủ quy tắc ReadMe của nhóm:
// - Tên biến/hằng bằng tiếng Anh rõ nghĩa, chú thích bằng tiếng Việt.
// - Tập trung toàn bộ thông số gameplay tại đây để dễ cân bằng và ghép code.
// =============================================================================
namespace Config {

    // -------------------------------------------------------------------------
    // 1. THÔNG SỐ BẢN ĐỒ & CỬA SỔ HIỂN THỊ (Khớp bảng phân công 24 x 18 ô)
    // -------------------------------------------------------------------------
    inline constexpr int MAP_COLS = 24;                 // Số cột của lưới bản đồ
    inline constexpr int MAP_ROWS = 18;                 // Số hàng của lưới bản đồ
    inline constexpr float TILE_SIZE = 40.0f;           // Kích thước mỗi ô vuông (pixel)
    inline constexpr float HUD_TOP_HEIGHT = 72.0f;      // Chiều cao thanh HUD phía trên bản đồ

    inline constexpr unsigned int WINDOW_WIDTH =
        static_cast<unsigned int>(MAP_COLS * TILE_SIZE); // 960 px
    inline constexpr unsigned int WINDOW_HEIGHT =
        static_cast<unsigned int>(MAP_ROWS * TILE_SIZE + HUD_TOP_HEIGHT); // 792 px
    inline constexpr unsigned int TARGET_FPS = 60;

    // -------------------------------------------------------------------------
    // 2. KỸ NĂNG PHÁ TƯỜNG (Dùng ở cả Màn 1 Sinh tồn & Màn 2 Đánh Boss)
    // Phụ trách: Huỳnh Nhựt Cường
    // -------------------------------------------------------------------------
    // Sử dụng phím Space (ngón cái tay trái) để nổ phá tường tức thì khi tay lái WASD
    inline constexpr sf::Keyboard::Key WALL_BREAK_KEY = sf::Keyboard::Key::Space;

    // Độ dài tối thiểu của rắn: khi rắn còn <= 3 đốt thì KHÔNG THỂ phá tường nữa
    // (Chỉ kích hoạt được khi rắn có từ 4 đốt trở lên)
    inline constexpr int MIN_SNAKE_LENGTH_FOR_WALL_BREAK = 3;

    // Chi phí mỗi lần kích hoạt phá tường: rắn bị ngắn đi đúng 1 đốt
    inline constexpr int WALL_BREAK_SEGMENT_COST = 1;

    // Bán kính vùng nổ theo số ô tính từ đầu rắn: 1 ô mỗi phía => vùng 3 x 3 ô
    inline constexpr int WALL_BREAK_HALF_EXTENT = 1;

    // Kỹ năng phá tường KHÔNG CÓ thời gian hồi chiêu (0.0 giây) -> bấm nổ liên tục tức thì!
    inline constexpr float WALL_BREAK_COOLDOWN = 0.0f;

    // Giới hạn độ dài tối đa của rắn (khớp thông số module người chơi)
    inline constexpr int MAX_SNAKE_LENGTH = 12;

    // -------------------------------------------------------------------------
    // 2B. RÀNG BUỘC SINH TÁO & TỐC ĐỘ XUẤT HIỆN TƯỜNG NGẪU NHIÊN
    // - Táo tuyệt đối không xuất hiện sát mép / góc tường bao (cách rìa >= 2 ô).
    // - Táo ưu tiên xuất hiện ngẫu nhiên gần đầu rắn hơn (từ 2 đến 6 ô).
    // - Tường ngẫu nhiên mọc nhanh hơn và tăng tốc dần theo số táo đã ăn.
    // -------------------------------------------------------------------------
    inline constexpr int APPLE_BORDER_MARGIN = 2;          // Cấm sinh táo ở col < 2, col >= 22, row < 2, row >= 16
    inline constexpr int APPLE_MIN_DIST_FROM_HEAD = 2;     // Cách đầu rắn tối thiểu 2 ô (không mọc đè ngay mũi rắn)
    inline constexpr int APPLE_MAX_DIST_FROM_HEAD = 6;     // Ưu tiên mọc gần trong phạm vi 6 ô quanh đầu rắn
    inline constexpr int APPLE_MAX_ADJACENT_WALLS = 1;     // Không sinh táo ở góc hẹp có >= 2 tường kề sát

    inline constexpr float WALL_SPAWN_BASE_INTERVAL = 1.15f;   // Thời gian cơ bản mọc 1 ô tường mới (nhanh gấp ~3 lần cũ)
    inline constexpr float WALL_SPAWN_ACCEL_PER_APPLE = 0.06f; // Mỗi quả táo ăn được làm tường mọc nhanh hơn 0.06s
    inline constexpr float WALL_SPAWN_MIN_INTERVAL = 0.55f;    // Ngưỡng mọc tường nhanh tối đa (0.55 giây/ô)

    // -------------------------------------------------------------------------
    // 3. HỆ THỐNG NGẮM BẮN BOSS & SLOW MOTION (CHỈ MỞ Ở MÀN 2 - ĐÁNH BOSS)
    // Phụ trách: Huỳnh Nhựt Cường
    // -------------------------------------------------------------------------
    // Nút Chuột Phải để bật/tắt (Toggle) chế độ ngắm bắn + Chuột Trái để khai hỏa
    inline constexpr sf::Mouse::Button AIM_TOGGLE_MOUSE_BUTTON = sf::Mouse::Button::Right;
    inline constexpr sf::Mouse::Button FIRE_SHOT_MOUSE_BUTTON = sf::Mouse::Button::Left;

    // Hệ số làm chậm thời gian thế giới khi đang ngắm: 10% tốc độ bình thường (timeScale = 0.10f)
    inline constexpr float SLOW_MOTION_FACTOR = 0.10f;
    inline constexpr float NORMAL_TIME_SCALE = 1.00f;

    // Bán kính vòng tròn ngắm tối đa quanh tâm đầu rắn: 5 ô
    inline constexpr float AIM_RANGE_TILES = 5.0f;
    inline constexpr float AIM_RANGE_PIXELS = AIM_RANGE_TILES * TILE_SIZE;

    // Sát thương gây ra cho Boss mỗi phát bắn trúng
    inline constexpr int BOSS_SHOT_DAMAGE = 15;

    // Số viên đạn tiêu hao cho mỗi phát bắn (kể cả trúng hay trượt / ngoài rìa 5 ô)
    inline constexpr int AMMO_COST_PER_SHOT = 1;

    // Giới hạn thời gian giữ ngắm mặc định cho 1 lần bật Space Toggle (10.0 giây)
    inline constexpr float MAX_AIM_DURATION = 10.0f;

    // Thời gian hồi chiêu ngay sau khi bắn 1 viên hoặc thoát chế độ ngắm (5 giây)
    inline constexpr float AIM_COOLDOWN_DURATION = 5.0f;

    // -------------------------------------------------------------------------
    // 4. THÔNG SỐ MẶC ĐỊNH CỦA BOSS (Dùng đồng bộ giữa module Boss & Sandbox)
    // -------------------------------------------------------------------------
    inline constexpr int BOSS_MAX_HP = 100;             // Máu tối đa của Boss
    inline constexpr int BOSS_SIZE_TILES = 2;           // Kích thước Boss: 2 x 2 ô

} // namespace Config
