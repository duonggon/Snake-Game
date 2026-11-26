#pragma once
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

// =============================================================================
// HỢP ĐỒNG GIAO TIẾP LIÊN MODULE (Combat/CombatInterfaces.h)
// Phụ trách: Huỳnh Nhựt Cường
//
// Tệp này định nghĩa các giao diện (Interface) ngắn gọn để module Chiến đấu
// kết nối trơn tru với code của 4 thành viên còn lại mà không phụ thuộc cứng:
// - Dương Vĩnh Gôn : Bản đồ 24x18 (ICombatMap) & Giai đoạn màn chơi (GameStage)
// - Đào Duy Khôi   : Rắn, độ dài đốt & đạn (ICombatPlayer)
// - Lê Đào Tấn Đạt : Quả cầu Boss 2x2 ô, 100 HP (ICombatBoss)
// - Lê Nhật Hoàng  : Âm thanh (CombatEventFlags) & Thống kê cuối trận (CombatStats)
// =============================================================================

// Hai giai đoạn chính của trò chơi:
// - Stage1_Survival : Màn 1 (Rắn ăn táo, tường mọc ngẫu nhiên -> CHỈ DÙNG PHÁ TƯỜNG PHÍM F)
// - Stage2_BossFight: Màn 2 (Xuất hiện Boss 2x2 -> DÙNG CẢ PHÁ TƯỜNG F & SÚNG SPACE + CHUỘT)
enum class GameStage {
    Stage1_Survival = 1,
    Stage2_BossFight = 2
};

// Giao diện truy xuất Bản đồ lưới 24 x 18 (module của Dương Vĩnh Gôn)
class ICombatMap {
public:
    virtual ~ICombatMap() = default;

    // Kiểm tra tọa độ ô (col, row) có nằm trong phạm vi hợp lệ [0..23] x [0..17] không
    virtual bool isInsideBounds(int col, int row) const = 0;

    // Kiểm tra ô (col, row) có phải là tường CÓ THỂ PHÁ HỦY không
    // (Trả về false đối với ô trống hoặc tường bao cố định FixedWall)
    virtual bool isDestructibleWall(int col, int row) const = 0;

    // Phá hủy tường tại ô (col, row), chuyển ô đó thành ô trống đi qua được
    virtual void destroyWall(int col, int row) = 0;

    // Đổi tọa độ ô lưới (col, row) sang tọa độ pixel tâm ô trong thế giới game
    virtual sf::Vector2f gridToWorldCenter(int col, int row) const = 0;
};

// Giao diện truy xuất Người chơi / Con rắn (module của Đào Duy Khôi)
class ICombatPlayer {
public:
    virtual ~ICombatPlayer() = default;

    // Kiểm tra rắn hiện còn sống hay đã đâm tường / mất mạng
    virtual bool isAlive() const = 0;

    // Lấy tọa độ ô lưới hiện tại của đốt đầu rắn
    virtual sf::Vector2i getHeadGridPos() const = 0;

    // Lấy tổng số đốt hiện tại của rắn (dùng làm điều kiện năng lượng phá tường: cần > 3 đốt)
    virtual int getSegmentCount() const = 0;

    // Cắt ngắn bớt 'count' đốt từ đuôi rắn sau khi kích hoạt phá tường
    virtual void shrinkSegments(int count) = 0;

    // Lấy số viên đạn hiện có (dùng ở Màn 2 khi bắn Boss)
    virtual int getAmmo() const = 0;

    // Trừ số viên đạn sau khi bắn (trả về true nếu trừ thành công)
    virtual bool consumeAmmo(int count) = 0;
};

// Giao diện truy xuất Boss 2x2 ô (module của Lê Đào Tấn Đạt)
class ICombatBoss {
public:
    virtual ~ICombatBoss() = default;

    // Boss đã xuất hiện trên bản đồ hay chưa
    virtual bool isActive() const = 0;

    // Boss còn sống (HP > 0) hay đã bị tiêu diệt
    virtual bool isAlive() const = 0;

    // Vùng va chạm (Hitbox 2x2 ô) của Boss trong hệ tọa độ thế giới (pixel)
    virtual sf::FloatRect getWorldHitbox() const = 0;

    // Trừ máu Boss khi trúng đạn; trả về true nếu phát bắn này kết liễu Boss (HP về 0)
    virtual bool takeDamage(int damageAmount) = 0;
};

// Cấu trúc lưu số liệu thống kê chiến đấu (phục vụ màn hình tổng kết của Lê Nhật Hoàng)
struct CombatStats {
    int totalWallsDestroyed = 0;   // Tổng số ô tường đã phá hủy
    int wallBreakUsedCount = 0;    // Số lần người chơi kích hoạt kỹ năng phá tường (Phím F)
    int shotsFired = 0;            // Tổng số viên đạn đã bắn ở Màn 2
    int shotsHit = 0;              // Số viên đạn bắn trúng Boss

    // Tính tỷ lệ bắn chính xác (0.0% -> 100.0%)
    float getAccuracyPercent() const {
        if (shotsFired <= 0) {
            return 0.0f;
        }
        return (static_cast<float>(shotsHit) * 100.0f) / static_cast<float>(shotsFired);
    }
};

// Cờ sự kiện phát sinh trong từng frame (để module Âm thanh của Lê Nhật Hoàng đọc & phát tiếng)
struct CombatEventFlags {
    bool wallBreakActivated = false; // Vừa bấm F nổ phá tường -> phát âm thanh nổ phá tường
    bool gunShotFired = false;       // Vừa bắn súng -> phát âm thanh bắn súng
    bool bossHit = false;            // Đạn vừa trúng Boss -> phát âm thanh Boss trúng đạn
    bool bossDefeated = false;       // Boss vừa nổ tung về 0 HP -> phát âm thanh chiến thắng
};
