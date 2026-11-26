#pragma once
#include <SFML/Graphics/Color.hpp>
#include <string_view>

// =============================================================================
// TỆP TÙY BIẾN ĐỒ HỌA & HIỆU ỨNG HÌNH ẢNH (Effects/VfxStyleConfig.h)
// Phụ trách: Huỳnh Nhựt Cường
//
// HƯỚNG DẪN TỰ CHỈNH SỬA ĐỒ HỌA THEO Ý THÍCH:
// 1. Mọi màu sắc đều viết dạng sf::Color{Đỏ, XanhLá, XanhDương, ĐộĐậmAlpha}
//    với giá trị từ 0 đến 255 (Alpha = 255 là đặc hoàn toàn, 0 là trong suốt).
// 2. Bạn chỉ cần đổi con số trong tệp này rồi bấm Ctrl+Shift+B (hoặc F5) trên
//    VS Code để xem ngay hiệu ứng mới mà không cần sửa sâu vào thuật toán.
// 3. Nếu bạn tự vẽ ảnh hiệu ứng (dải ngang Sprite Sheet PNG) hoặc tải trên mạng,
//    chỉ cần chép file ảnh vào thư mục Assets/VFX/ đúng tên bên dưới — hệ thống
//    sẽ tự động ưu tiên hiển thị ảnh của bạn!
// =============================================================================
namespace VfxStyle {

    // =========================================================================
    // PHẦN A: KHE CẮM ẢNH SPRITE SHEET TÙY CHỌN (PNG DẢI NGANG - HORIZONTAL STRIP)
    // - Nếu chưa có file ảnh trong Assets/VFX/, game tự động vẽ bằng hình học SFML
    //   mượt mà (không bao giờ báo lỗi thiếu file).
    // - Khi bạn bỏ file ảnh đúng đường dẫn vào, game tự động chuyển sang vẽ ảnh!
    // =========================================================================

    // Ảnh hiệu ứng gạch vỡ (hỗ trợ cả dải ngang 1 hàng LẪN bảng nhiều hàng x cột!)
    // Ví dụ: ảnh có 6 khung nằm trên 1 hàng ngang -> COLS = 6, ROWS = 1
    //        ảnh bảng 4x3 (12 khung) tải trên itch.io -> COLS = 4, ROWS = 3
    inline constexpr std::string_view WALL_BREAK_SPRITESHEET = "Assets/VFX/wall_break.png";
    inline constexpr int WALL_BREAK_SPRITE_COLS = 6;
    inline constexpr int WALL_BREAK_SPRITE_ROWS = 1;

    // Ảnh hiệu ứng vụ nổ khi Boss bị tiêu diệt (hỗ trợ bảng COLS x ROWS bất kỳ)
    inline constexpr std::string_view BOSS_EXPLOSION_SPRITESHEET = "Assets/VFX/boss_explosion.png";
    inline constexpr int BOSS_EXPLOSION_SPRITE_COLS = 12;
    inline constexpr int BOSS_EXPLOSION_SPRITE_ROWS = 1;

    // Ảnh hiệu ứng đầu đạn (tùy chọn 1 ảnh đơn PNG nhỏ cho viên đạn)
    inline constexpr std::string_view BULLET_SPRITE_IMAGE = "Assets/VFX/bullet.png";

    // =========================================================================
    // PHẦN B: HIỆU ỨNG GẠCH VỠ & BIẾN MẤT DẦN (Khi nhấn phím F phá tường)
    // Yêu cầu: Gạch chỉ tách vỡ nhẹ tại chỗ rồi mờ dần biến mất trong 0.5 giây.
    // =========================================================================

    // Thời gian mảnh gạch mờ dần cho tới khi biến mất hoàn toàn (giây) -> Mặc định 0.5s
    inline constexpr float WALL_CRUMBLE_DURATION = 0.50f;

    // Số mảnh gạch nhỏ tách ra bên trong 1 ô tường bị phá (nên từ 4 đến 9 mảnh)
    inline constexpr int WALL_CRACK_ROWS = 2;
    inline constexpr int WALL_CRACK_COLS = 2; // 2 x 2 = 4 mảnh gạch chính + 2 mảnh vụn nhỏ
    inline constexpr int WALL_EXTRA_CHIPS = 2;

    // Tốc độ các mảnh gạch nứt tách nhẹ ra khỏi tâm ô gạch (pixel/giây)
    // Giữ số nhỏ (20..40) để gạch vỡ gọn tại chỗ, không văng loạn xạ khắp bản đồ
    inline constexpr float WALL_CRACK_SPREAD_SPEED = 26.0f;

    // Tốc độ xoay nhẹ của mảnh gạch khi vỡ (độ/giây)
    inline constexpr float WALL_CRACK_ROTATION_SPEED = 85.0f;

    // Màu sắc mảnh gạch vỡ (Nâu gạch ấm + viền sáng nhẹ lúc mới nứt)
    inline constexpr sf::Color WALL_PIECE_MAIN_COLOR{198, 106, 56, 255};
    inline constexpr sf::Color WALL_PIECE_ALT_COLOR{162, 82, 42, 255};
    inline constexpr sf::Color WALL_CRACK_FLASH_COLOR{255, 220, 140, 190};

    // Vòng xung lực nhẹ báo hiệu phạm vi nổ 3x3 quanh đầu rắn khi nhấn phím F
    inline constexpr float SKILL_PULSE_DURATION = 0.28f;
    inline constexpr sf::Color SKILL_PULSE_OUTLINE_COLOR{255, 195, 75, 210};
    inline constexpr sf::Color SKILL_PULSE_FILL_COLOR{255, 175, 50, 38};

    // =========================================================================
    // PHẦN C: HIỆU ỨNG ĐƯỜNG ĐẠN NÉT ĐỨT & VÒNG NGẮM 5 Ô (Chỉ có ở Màn 2 - Boss)
    // Yêu cầu: Đường đạn là một đường nét đứt nối từ đầu rắn đến viên đạn.
    // =========================================================================

    // Tốc độ viên đạn bay từ đầu rắn tới mục tiêu / rìa 5 ô (pixel/giây)
    inline constexpr float BULLET_PROJECTILE_SPEED = 1100.0f;

    // Bán kính viên đạn tròn phát sáng (pixel)
    inline constexpr float BULLET_HEAD_RADIUS = 5.5f;

    // Cấu hình ĐƯỜNG NÉT ĐỨT (Dashed Line) nối từ đầu rắn -> viên đạn:
    inline constexpr float BULLET_DASH_LENGTH = 11.0f;      // Chiều dài mỗi vạch nét đứt (px)
    inline constexpr float BULLET_DASH_GAP = 7.0f;          // Khoảng hở giữa 2 vạch (px)
    inline constexpr float BULLET_DASH_THICKNESS = 3.2f;    // Độ dày của vạch nét đứt (px)

    // Thời gian đường nét đứt lưu lại và mờ dần sau khi viên đạn chạm đích (giây)
    inline constexpr float BULLET_TRAIL_FADE_DURATION = 0.26f;

    // Màu sắc viên đạn & đường nét đứt khi TRÚNG Boss vs khi TRƯỢT
    inline constexpr sf::Color BULLET_HEAD_COLOR{255, 250, 190, 255};
    inline constexpr sf::Color DASH_TRAIL_HIT_COLOR{255, 220, 70, 235};   // Vàng rực khi trúng
    inline constexpr sf::Color DASH_TRAIL_MISS_COLOR{185, 215, 240, 195}; // Xanh trắng dịu khi trượt

    // Cấu hình Vòng bán kính ngắm 5 ô & Đường ngắm xem trước khi đang bật Space:
    inline constexpr sf::Color AIM_RING_FILL_COLOR{65, 185, 255, 22};
    inline constexpr sf::Color AIM_RING_OUTLINE_COLOR{95, 215, 255, 175};
    inline constexpr float AIM_RING_OUTLINE_THICKNESS = 2.0f;

    // Màu đường nét đứt xem trước lúc đang lia chuột ngắm (trước khi bấm bắn)
    inline constexpr sf::Color AIM_PREVIEW_DASH_IN_RANGE{130, 245, 170, 155};  // Chuột trong vòng 5 ô
    inline constexpr sf::Color AIM_PREVIEW_DASH_CLAMPED{255, 165, 90, 155};    // Chuột vượt ngoài 5 ô (bị chặn ở rìa)
    inline constexpr sf::Color AIM_CROSSHAIR_LOCK_BOSS{255, 85, 85, 235};      // Đỏ cam khi tâm ngắm khóa trúng Boss

    // =========================================================================
    // PHẦN D: HIỂN THỊ SỐ SÁT THƯƠNG BAY LÊN ("-15" hoặc "MISS")
    // =========================================================================
    inline constexpr unsigned int DAMAGE_FONT_SIZE_HIT = 24;
    inline constexpr unsigned int DAMAGE_FONT_SIZE_MISS = 17;
    inline constexpr float DAMAGE_TEXT_LIFETIME = 0.75f;     // Thời gian tồn tại chữ nổi (giây)
    inline constexpr float DAMAGE_TEXT_RISE_SPEED = 52.0f;   // Tốc độ bay lên (pixel/giây)
    inline constexpr sf::Color DAMAGE_TEXT_HIT_FILL{255, 80, 70, 255};
    inline constexpr sf::Color DAMAGE_TEXT_HIT_OUTLINE{30, 10, 10, 230};
    inline constexpr sf::Color DAMAGE_TEXT_MISS_FILL{200, 210, 220, 215};

    // =========================================================================
    // PHẦN E: RUNG MÀN HÌNH NHẸ + TÍCH LŨY CẤP SỐ NHÂN (Giới hạn tối đa 6/10)
    // Yêu cầu:
    // - Rung nhẹ nhàng, nếu kích hoạt liên tiếp thì cộng dồn mạnh dần nhưng
    //   TỐI ĐA CHỈ Ở MỨC 6/10.
    // - Thời gian lần đầu u1 = 1.5 giây; nếu kích hoạt dồn thêm khi đang rung
    //   thì cộng dồn theo cấp số nhân với công bội q = 30% (0.30).
    // =========================================================================
    inline constexpr bool ENABLE_SCREEN_SHAKE = true;        // Đổi thành false nếu muốn tắt hẳn rung

    // Quy đổi thang điểm 10 sang độ lệch pixel cực đại:
    // Mức 10/10 tương ứng 10.0 px => Mức trần 6/10 chỉ dịch tối đa ~6.0 px (rất êm, không chóng mặt)
    inline constexpr float SHAKE_PIXELS_PER_LEVEL = 1.0f;

    inline constexpr float SHAKE_INITIAL_LEVEL = 2.0f;       // Lần rung đầu tiên rất nhẹ: mức 2.0 / 10
    inline constexpr float SHAKE_STACK_ADD_LEVEL = 1.25f;    // Mỗi lần kích hoạt liên tiếp cộng thêm +1.25 mức
    inline constexpr float SHAKE_MAX_LEVEL_CAP = 6.0f;       // TRẦN CỨNG: Tối đa mức 6.0 / 10

    inline constexpr float SHAKE_FIRST_TERM_SECONDS = 1.50f; // u1 = 1.5 giây
    inline constexpr float SHAKE_GEOMETRIC_RATIO = 0.30f;    // Công bội q = 30% = 0.30

    // =========================================================================
    // PHẦN F: HIỆU ỨNG MÀU MÀN HÌNH KHI SLOW MOTION (Mờ dần mượt mà)
    // =========================================================================
    // Màu phủ toàn màn hình khi bật Slow Motion 30% (Alpha = 88 để nhìn rõ bản đồ)
    inline constexpr sf::Color SLOW_MO_SCREEN_TINT{16, 42, 88, 88};
    // Viền khung cảnh sáng xanh băng giá nhẹ ở 4 cạnh màn hình khi đang Slow-Mo
    inline constexpr sf::Color SLOW_MO_BORDER_GLOW{80, 195, 255, 120};

    // Tốc độ chuyển mượt Fade-In (khi bật Space) và Fade-Out (khi thoát Space)
    // Giá trị 5.0f => chuyển mượt từ 0% lên 100% trong khoảng 0.20 giây
    inline constexpr float SLOW_MO_FADE_IN_RATE = 5.5f;
    inline constexpr float SLOW_MO_FADE_OUT_RATE = 4.5f;

    // =========================================================================
    // PHẦN G: HIỆU ỨNG NỔ KHI BOSS BỊ TIÊU DIỆT (Chủ thể nổ: Quả cầu Boss 2x2 ô)
    // =========================================================================
    inline constexpr float BOSS_EXPLOSION_TOTAL_DURATION = 1.15f; // Tổng thời gian vụ nổ Boss
    inline constexpr int BOSS_EXPLOSION_RING_COUNT = 4;           // Số đợt vòng sóng xung kích lệch pha
    inline constexpr int BOSS_EXPLOSION_SPARK_COUNT = 56;         // Số hạt năng lượng Boss văng 360 độ
    inline constexpr float BOSS_SPARK_MIN_SPEED = 75.0f;
    inline constexpr float BOSS_SPARK_MAX_SPEED = 290.0f;
    inline constexpr sf::Color BOSS_CORE_FLASH_COLOR{255, 245, 200, 245};
    inline constexpr sf::Color BOSS_SPARK_PRIMARY_COLOR{255, 110, 55, 255};
    inline constexpr sf::Color BOSS_SPARK_SECONDARY_COLOR{255, 215, 75, 255};

    // (Tùy chọn tặng kèm) Hiệu ứng tan rã nhỏ khi Rắn đâm vào tường bị DIE
    inline constexpr bool ENABLE_SNAKE_DEATH_BURST = true;
    inline constexpr sf::Color SNAKE_DEATH_PARTICLE_COLOR{110, 235, 130, 240};

} // namespace VfxStyle
