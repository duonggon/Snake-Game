#pragma once
#include "Combat/CombatInterfaces.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <optional>

class EffectManager;
class ScreenShake;

// =============================================================================
// HỆ THỐNG CHIẾN ĐẤU TRUNG TÂM (Combat/CombatSystem.h)
// Phụ trách: Huỳnh Nhựt Cường
//
// Quản lý trọn vẹn 2 cơ chế đặc trưng của game:
// 1. Kỹ năng phá tường (Phím F - Hoạt động ở cả Màn 1 & Màn 2):
//    - Kiểm tra rắn có > 3 đốt (tối thiểu 3 đốt thì khóa kỹ năng).
//    - Nổ vùng 3x3 quanh đầu rắn, trừ ngay 1 đốt, phá tường hợp lệ, gạch vỡ 0.5s.
// 2. Hệ thống ngắm bắn Boss (Phím Space Toggle + Chuột trái - CHỈ MỞ Ở MÀN 2):
//    - Bật Slow Motion 30%, hiển thị vòng ngắm 5 ô + đường ngắm nét đứt.
//    - Bắn ngoài vòng 5 ô vẫn tốn 1 đạn và bay tối đa tới rìa 5 ô.
//    - Trúng Boss 2x2 gây 15 HP sát thương; bắn xong 1 viên tự thoát ngay + hồi chiêu 5s.
// =============================================================================
class CombatSystem {
public:
    // Trạng thái của hệ thống ngắm bắn Boss
    enum class AimState {
        Ready,      // Sẵn sàng bật Space (khi đang ở Màn 2 & còn đạn)
        Aiming,     // Đang ngắm bắn (Slow Motion 30% đang kích hoạt)
        Cooldown    // Đang hồi chiêu 5.0 giây (khóa tạm thời Slow Motion)
    };

    // Thông tin kết quả của một phát bắn (phục vụ kiểm thử & phản hồi UI)
    struct ShotResult {
        bool fired = false;             // Súng có thực sự khai hỏa hay không
        bool hitBoss = false;           // Viên đạn có chạm trúng Boss 2x2 không
        bool killedBoss = false;        // Phát đạn có tiêu diệt Boss về 0 HP không
        bool clampedToMaxRange = false; // Chuột có nằm ngoài tầm 5 ô và bị kẹp về rìa không
        sf::Vector2f originWorld{};     // Tọa độ tâm đầu rắn lúc bắn
        sf::Vector2f impactWorld{};     // Điểm dừng thực tế của viên đạn (trên Boss hoặc rìa 5 ô)
    };

    CombatSystem();

    // Đặt lại toàn bộ trạng thái chiến đấu (khi bắt đầu ván mới)
    void resetAll(GameStage initialStage = GameStage::Stage1_Survival);

    // Chuyển đổi giai đoạn màn chơi (Màn 1 Sinh tồn <-> Màn 2 Đánh Boss)
    void setGameStage(GameStage stage, EffectManager* effects = nullptr);
    GameStage getGameStage() const;

    // -------------------------------------------------------------------------
    // 1. KỸ NĂNG PHÁ TƯỜNG (PHÍM F - DÙNG Ở CẢ MÀN 1 & MÀN 2)
    // -------------------------------------------------------------------------
    bool canUseWallBreak(const ICombatPlayer& player) const;

    // Kích hoạt phá tường 3x3 quanh đầu rắn; trả về số ô tường thực tế bị phá hủy
    // (Trả về -1 nếu không đủ điều kiện kích hoạt do rắn <= 3 đốt hoặc rắn đã chết)
    int tryActivateWallBreak(
        ICombatMap& map,
        ICombatPlayer& player,
        EffectManager& effects,
        ScreenShake& screenShake
    );

    // -------------------------------------------------------------------------
    // 2. HỆ THỐNG NGẮM BẮN BOSS (SPACE TOGGLE + CHUỘT TRÁI - CHỈ Ở MÀN 2)
    // -------------------------------------------------------------------------
    // Xử lý khi người chơi nhấn phím Space (Toggle bật hoặc tắt chế độ ngắm)
    bool tryToggleAim(
        const ICombatPlayer& player,
        const ICombatBoss* boss,
        EffectManager& effects
    );

    // Xử lý khi người chơi click Chuột trái để khai hỏa (mouseWorldPos lấy qua View tĩnh)
    ShotResult tryFireShot(
        sf::Vector2f mouseWorldPos,
        const ICombatMap& map,
        ICombatPlayer& player,
        ICombatBoss* boss,
        EffectManager& effects,
        ScreenShake& screenShake
    );

    // Cập nhật bộ đếm thời gian ngắm, hồi chiêu 5s theo thời gian thực (unscaledDeltaTime)
    void update(
        float unscaledDeltaTime,
        const ICombatPlayer& player,
        const ICombatBoss* boss,
        EffectManager& effects
    );

    // Vẽ vòng bán kính ngắm 5 ô, đường ngắm nét đứt xem trước và tâm ngắm Crosshair
    void renderAimingWorldUI(
        sf::RenderTarget& target,
        sf::Vector2f mouseWorldPos,
        const ICombatMap& map,
        const ICombatPlayer& player,
        const ICombatBoss* boss
    ) const;

    // -------------------------------------------------------------------------
    // TRUY VẤN TRẠNG THÁI, HỆ SỐ THỜI GIAN SLOW-MO & THỐNG KÊ
    // -------------------------------------------------------------------------
    // Hệ số nhân thời gian cho Game Loop: 0.30f khi đang Aiming, 1.00f ở mọi lúc khác
    float getTimeScale() const;

    AimState getAimState() const;
    bool isAiming() const;
    float getAimTimeRemaining() const;
    float getCooldownRemaining() const;

    const CombatStats& getStats() const;
    const CombatEventFlags& getLastFrameEvents() const;
    void clearFrameEvents();

    // Hàm toán học tĩnh: Tính điểm đích bị kẹp ở bán kính 5 ô & kiểm tra cắt Hitbox Boss
    static sf::Vector2f clampTargetToRange(
        sf::Vector2f originWorld,
        sf::Vector2f mouseWorld,
        float maxRangePx,
        bool* outWasClamped = nullptr
    );

    static std::optional<float> intersectSegmentWithRect(
        sf::Vector2f segStart,
        sf::Vector2f segEnd,
        const sf::FloatRect& rect
    );

private:
    GameStage m_currentStage;
    AimState m_aimState;
    float m_aimTimeRemaining;
    float m_cooldownRemaining;
    CombatStats m_stats;
    CombatEventFlags m_frameEvents;

    void enterCooldownState(EffectManager* effects);
};
