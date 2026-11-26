#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <random>
#include <string>
#include <vector>

// =============================================================================
// LỚP QUẢN LÝ TOÀN BỘ HIỆU ỨNG HÌNH ẢNH (Effects/EffectManager.h)
// Phụ trách: Huỳnh Nhựt Cường
//
// Bao gồm đầy đủ các hiệu ứng trong bảng phân công:
// 1. Hiệu ứng gạch vỡ mờ dần biến mất trong 0.5 giây (+ xung lực 3x3 quanh đầu rắn).
// 2. Hiệu ứng đường đạn nét đứt nối từ tâm đầu rắn đến viên đạn (Màn 2 - Boss).
// 3. Hiển thị số sát thương bay lên ("-15" hoặc "MISS").
// 4. Hiệu ứng phủ màu màn hình mờ vào / mờ ra mượt mà khi bật Slow Motion 30%.
// 5. Hiệu ứng vụ nổ lớn khi Boss (2x2 ô) bị tiêu diệt về 0 HP.
// 6. Hỗ trợ tự động nạp Sprite Sheet PNG từ Assets/VFX/ nếu người dùng muốn thay ảnh!
// =============================================================================
class EffectManager {
public:
    EffectManager();

    // Thử nạp font chữ mặc định và các file ảnh Sprite Sheet tùy chọn trong Assets/VFX/
    void loadOptionalAssets();

    // Cho phép truyền font dùng chung từ hệ thống UI của nhóm (nếu có)
    void setSharedFont(const sf::Font* sharedFont);

    // Xóa toàn bộ hiệu ứng đang tồn tại trên màn hình (khi chơi lại / chuyển màn)
    void clearAll();

    // -------------------------------------------------------------------------
    // NHÓM HÀM PHÁT SINH HIỆU ỨNG (Gọi từ CombatSystem hoặc Game)
    // -------------------------------------------------------------------------

    // 1a. Chớp khung vuông chuẩn phạm vi nổ (mặc định 3x3 ô) quanh đầu rắn khi bấm phím F
    void spawnWallSkillPulse(sf::Vector2f snakeHeadWorldPos, float areaWidthPx);

    // 1b. Hiệu ứng 1 ô tường gạch nứt vỡ tại chỗ rồi mờ dần biến mất trong 0.5 giây
    void spawnWallCrumbleEffect(sf::Vector2f tileCenterWorldPos, float tileSizePx);

    // 2. Hiệu ứng bắn đạn ở Màn 2: viên đạn bay từ đầu rắn tới đích kèm đường nét đứt
    void spawnDashedBulletShot(
        sf::Vector2f snakeHeadWorldPos,
        sf::Vector2f targetWorldPos,
        bool isHitBoss
    );

    // 3. Hiển thị số sát thương ("-15") khi trúng Boss hoặc chữ ("MISS") khi trượt
    void spawnFloatingDamageText(sf::Vector2f worldPos, int damageAmount, bool isHitBoss);

    // 4. Cập nhật mục tiêu bật/tắt lớp màu màn hình Slow Motion (tự động fade mượt)
    void setSlowMotionVisualActive(bool active);

    // 5. Hiệu ứng nổ tung quả cầu Boss (2x2 ô) khi HP của Boss giảm về 0
    void spawnBossDeathExplosion(sf::Vector2f bossCenterWorldPos, float bossDiameterPx);

    // 6. (Tặng kèm) Hiệu ứng hạt tan rã khi rắn đâm trúng tường bị DIE
    void spawnSnakeDeathBurst(sf::Vector2f snakeHeadWorldPos);

    // -------------------------------------------------------------------------
    // NHÓM HÀM CẬP NHẬT & VẼ ĐỒ HỌA
    // -------------------------------------------------------------------------

    // Cập nhật toàn bộ hiệu ứng theo thời gian thực (unscaledDeltaTime)
    void update(float unscaledDeltaTime);

    // Vẽ các hiệu ứng nằm trong không gian thế giới bản đồ (gạch vỡ, đường đạn, nổ Boss, chữ nổi)
    void renderWorldEffects(sf::RenderTarget& target) const;

    // Vẽ lớp phủ màu màn hình Slow Motion mượt mà (gọi ở View giao diện tĩnh cuối frame)
    void renderScreenOverlay(sf::RenderTarget& target, sf::Vector2f screenSize) const;

    // Hàm tiện ích tĩnh: Vẽ một đường nét đứt giữa 2 điểm bất kỳ (dùng cho cả tia ngắm & đường đạn)
    static void drawDashedLine(
        sf::RenderTarget& target,
        sf::Vector2f startPos,
        sf::Vector2f endPos,
        float dashLength,
        float gapLength,
        float thickness,
        sf::Color color
    );

    // --- Các hàm truy vấn phục vụ kiểm thử (Unit Tests) ---
    std::size_t getActiveWorldEffectCount() const;
    float getSlowMotionOverlayBlend() const; // 0.0 (tắt hẳn) -> 1.0 (hiện đầy đủ)

private:
    // --- Cấu trúc mảnh gạch vỡ mờ dần tại chỗ ---
    struct BrickPieceEffect {
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Vector2f size;
        float rotationDeg{};
        float angularSpeedDeg{};
        float elapsed{};
        float duration{};
        sf::Color baseColor;
    };

    // --- Cấu trúc vòng sóng xung kích (dùng cho xung 3x3 & nổ Boss) ---
    struct ShockwaveRingEffect {
        sf::Vector2f center;
        float startRadius{};
        float endRadius{};
        float delay{};       // Cho phép tạo nhiều đợt sóng nổ lệch pha khi Boss chết
        float elapsed{};
        float duration{};
        float outlineThickness{};
        sf::Color outlineColor;
        sf::Color fillColor;
    };

    // --- Cấu trúc hạt sáng văng tự do (dùng cho vụ nổ tiêu diệt Boss & rắn die) ---
    struct SparkParticleEffect {
        sf::Vector2f position;
        sf::Vector2f velocity;
        float radius{};
        float drag{};        // Lực cản không khí giúp hạt nổ chậm dần tự nhiên
        float elapsed{};
        float duration{};
        sf::Color baseColor;
    };

    // --- Cấu trúc viên đạn + đường nét đứt từ đầu rắn đến viên đạn ---
    struct DashedBulletEffect {
        sf::Vector2f originPos;
        sf::Vector2f targetPos;
        sf::Vector2f currentBulletPos;
        float speed{};
        bool reachedTarget{};
        bool isHit{};
        float fadeElapsed{};
        float fadeDuration{};
    };

    // --- Cấu trúc chữ số sát thương bay lên ---
    struct FloatingTextEffect {
        sf::Vector2f position;
        float riseSpeed{};
        float elapsed{};
        float duration{};
        unsigned int fontSize{};
        std::string label;
        sf::Color fillColor;
        sf::Color outlineColor;
    };

    // --- Cấu trúc hoạt ảnh Sprite Sheet PNG (hỗ trợ cả dải ngang 1 hàng & lưới nhiều hàng) ---
    struct SpriteStripEffect {
        const sf::Texture* texture{};
        sf::Vector2f center;
        float targetSizePx{};
        int sheetCols{1};
        int sheetRows{1};
        float elapsed{};
        float duration{};
    };

    // --- Cấu trúc khung vuông chớp sáng phạm vi 3x3 quanh đầu rắn ---
    struct SquarePulseEffect {
        sf::Vector2f center;
        float startSizePx{};
        float endSizePx{};
        float elapsed{};
        float duration{};
        sf::Color outlineColor;
        sf::Color fillColor;
    };

    // Dữ liệu font & texture tùy chọn
    sf::Font m_ownedFont;
    bool m_hasOwnedFont;
    const sf::Font* m_externalFont;

    sf::Texture m_wallBreakSheet;
    bool m_hasWallBreakSheet;

    sf::Texture m_bossExplosionSheet;
    bool m_hasBossExplosionSheet;

    sf::Texture m_bulletTexture;
    bool m_hasBulletTexture;

    // Trạng thái làm mờ mượt màn hình Slow Motion
    bool m_slowMoTargetActive;
    float m_slowMoBlend; // Chạy mượt trong đoạn [0.0f, 1.0f]

    // Danh sách các hiệu ứng đang hoạt động
    std::vector<BrickPieceEffect> m_brickPieces;
    std::vector<ShockwaveRingEffect> m_shockwaves;
    std::vector<SquarePulseEffect> m_squarePulses;
    std::vector<SparkParticleEffect> m_sparks;
    std::vector<DashedBulletEffect> m_bullets;
    std::vector<FloatingTextEffect> m_floatingTexts;
    std::vector<SpriteStripEffect> m_spriteAnims;

    std::mt19937 m_rng;

    const sf::Font* resolveActiveFont() const;
    static sf::Color scaleColorAlpha(sf::Color base, float factor01);
};
