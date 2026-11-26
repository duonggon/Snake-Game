#include "Effects/EffectManager.h"
#include "Effects/VfxStyleConfig.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Angle.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>

namespace {
    constexpr float PI_CONST = 3.1415926535f;
}

EffectManager::EffectManager()
    : m_hasOwnedFont(false),
      m_externalFont(nullptr),
      m_hasWallBreakSheet(false),
      m_hasBossExplosionSheet(false),
      m_hasBulletTexture(false),
      m_slowMoTargetActive(false),
      m_slowMoBlend(0.0f),
      m_rng(20261126u) {}

void EffectManager::loadOptionalAssets() {
    // 1. Nạp font chữ hiển thị số sát thương ("-15" / "MISS"):
    //    Tìm trong thư mục dự án trước, nếu chưa có thì lấy font chuẩn sẵn có của Windows.
    const std::vector<std::filesystem::path> candidateFonts = {
        "Assets/Fonts/game_font.ttf",
        "C:/Windows/Fonts/consolab.ttf",
        "C:/Windows/Fonts/arialbd.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };
    for (const auto& fontPath : candidateFonts) {
        std::error_code ec;
        if (std::filesystem::exists(fontPath, ec) && m_ownedFont.openFromFile(fontPath)) {
            m_hasOwnedFont = true;
            break;
        }
    }

    // 2. Tự động kiểm tra các file ảnh hiệu ứng tùy chọn trong Assets/VFX/:
    //    Nếu bạn tự vẽ ảnh bỏ vào đúng đường dẫn trong VfxStyleConfig.h, hệ thống sẽ bật chế độ vẽ ảnh!
    std::error_code ec;
    const std::filesystem::path wallSheetPath(VfxStyle::WALL_BREAK_SPRITESHEET);
    if (!VfxStyle::WALL_BREAK_SPRITESHEET.empty() && std::filesystem::exists(wallSheetPath, ec)) {
        m_hasWallBreakSheet = m_wallBreakSheet.loadFromFile(wallSheetPath);
    }

    const std::filesystem::path bossSheetPath(VfxStyle::BOSS_EXPLOSION_SPRITESHEET);
    if (!VfxStyle::BOSS_EXPLOSION_SPRITESHEET.empty() && std::filesystem::exists(bossSheetPath, ec)) {
        m_hasBossExplosionSheet = m_bossExplosionSheet.loadFromFile(bossSheetPath);
    }

    const std::filesystem::path bulletImgPath(VfxStyle::BULLET_SPRITE_IMAGE);
    if (!VfxStyle::BULLET_SPRITE_IMAGE.empty() && std::filesystem::exists(bulletImgPath, ec)) {
        m_hasBulletTexture = m_bulletTexture.loadFromFile(bulletImgPath);
    }
}

void EffectManager::setSharedFont(const sf::Font* sharedFont) {
    m_externalFont = sharedFont;
}

void EffectManager::clearAll() {
    m_brickPieces.clear();
    m_shockwaves.clear();
    m_squarePulses.clear();
    m_sparks.clear();
    m_bullets.clear();
    m_floatingTexts.clear();
    m_spriteAnims.clear();
    m_slowMoTargetActive = false;
    m_slowMoBlend = 0.0f;
}

// =============================================================================
// 1. HIỆU ỨNG PHÁ TƯỜNG: KHUNG VUÔNG CHUẨN 3x3 & GẠCH VỠ MỜ DẦN TRONG 0.5 GIÂY
// =============================================================================
void EffectManager::spawnWallSkillPulse(sf::Vector2f snakeHeadWorldPos, float areaWidthPx) {
    SquarePulseEffect boxPulse;
    boxPulse.center = snakeHeadWorldPos;
    boxPulse.startSizePx = areaWidthPx * 0.45f;
    boxPulse.endSizePx = areaWidthPx; // Khớp chính xác kích thước 3x3 ô (3 * TILE_SIZE)
    boxPulse.elapsed = 0.0f;
    boxPulse.duration = VfxStyle::SKILL_PULSE_DURATION;
    boxPulse.outlineColor = VfxStyle::SKILL_PULSE_OUTLINE_COLOR;
    boxPulse.fillColor = VfxStyle::SKILL_PULSE_FILL_COLOR;
    m_squarePulses.push_back(boxPulse);
}

void EffectManager::spawnWallCrumbleEffect(sf::Vector2f tileCenterWorldPos, float tileSizePx) {
    // [CHẾ ĐỘ ẢNH SPRITE SHEET]: Nếu bạn đã chép file Assets/VFX/wall_break.png vào, ưu tiên phát ảnh đó
    if (m_hasWallBreakSheet &&
        VfxStyle::WALL_BREAK_SPRITE_COLS > 0 &&
        VfxStyle::WALL_BREAK_SPRITE_ROWS > 0) {
        SpriteStripEffect anim;
        anim.texture = &m_wallBreakSheet;
        anim.center = tileCenterWorldPos;
        anim.targetSizePx = tileSizePx * 1.30f;
        anim.sheetCols = VfxStyle::WALL_BREAK_SPRITE_COLS;
        anim.sheetRows = VfxStyle::WALL_BREAK_SPRITE_ROWS;
        anim.elapsed = 0.0f;
        anim.duration = VfxStyle::WALL_CRUMBLE_DURATION;
        m_spriteAnims.push_back(anim);
        return;
    }

    // [CHẾ ĐỘ VẼ CODE MẶC ĐỊNH]:
    // Chia ô gạch thành các mảnh nứt nhẹ tại chỗ và biến mất dần trong đúng 0.5 giây (WALL_CRUMBLE_DURATION)
    const int rows = std::max(1, VfxStyle::WALL_CRACK_ROWS);
    const int cols = std::max(1, VfxStyle::WALL_CRACK_COLS);
    const float subW = (tileSizePx * 0.82f) / static_cast<float>(cols);
    const float subH = (tileSizePx * 0.82f) / static_cast<float>(rows);

    std::uniform_real_distribution<float> speedJitter(0.75f, 1.25f);
    std::uniform_real_distribution<float> rotDirDist(-1.0f, 1.0f);

    // Chớp sáng vết nứt cực ngắn ngay tâm ô gạch lúc vừa vỡ
    ShockwaveRingEffect crackFlash;
    crackFlash.center = tileCenterWorldPos;
    crackFlash.startRadius = tileSizePx * 0.15f;
    crackFlash.endRadius = tileSizePx * 0.52f;
    crackFlash.delay = 0.0f;
    crackFlash.elapsed = 0.0f;
    crackFlash.duration = VfxStyle::WALL_CRUMBLE_DURATION * 0.45f;
    crackFlash.outlineThickness = 2.0f;
    crackFlash.outlineColor = VfxStyle::WALL_CRACK_FLASH_COLOR;
    crackFlash.fillColor = sf::Color(255, 220, 140, 35);
    m_shockwaves.push_back(crackFlash);

    // Sinh các mảnh gạch chính tách nhẹ từ vị trí ô tường rồi mờ dần biến mất
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const float relX = (static_cast<float>(c) + 0.5f - static_cast<float>(cols) * 0.5f) * subW;
            const float relY = (static_cast<float>(r) + 0.5f - static_cast<float>(rows) * 0.5f) * subH;

            sf::Vector2f dir{relX, relY};
            const float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len > 0.001f) {
                dir /= len;
            }

            BrickPieceEffect piece;
            piece.position = {tileCenterWorldPos.x + relX, tileCenterWorldPos.y + relY};
            piece.velocity = dir * (VfxStyle::WALL_CRACK_SPREAD_SPEED * speedJitter(m_rng));
            piece.size = {subW * 0.90f, subH * 0.90f};
            piece.rotationDeg = rotDirDist(m_rng) * 8.0f;
            piece.angularSpeedDeg = rotDirDist(m_rng) * VfxStyle::WALL_CRACK_ROTATION_SPEED;
            piece.elapsed = 0.0f;
            piece.duration = VfxStyle::WALL_CRUMBLE_DURATION; // Mặc định 0.50 giây
            piece.baseColor = ((r + c) % 2 == 0)
                                  ? VfxStyle::WALL_PIECE_MAIN_COLOR
                                  : VfxStyle::WALL_PIECE_ALT_COLOR;
            m_brickPieces.push_back(piece);
        }
    }

    // Thêm vài mảnh vụn nhỏ ở tâm để vết vỡ tự nhiên hơn
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * PI_CONST);
    for (int i = 0; i < VfxStyle::WALL_EXTRA_CHIPS; ++i) {
        const float angle = angleDist(m_rng);
        BrickPieceEffect chip;
        chip.position = tileCenterWorldPos;
        chip.velocity = {
            std::cos(angle) * VfxStyle::WALL_CRACK_SPREAD_SPEED * 1.15f,
            std::sin(angle) * VfxStyle::WALL_CRACK_SPREAD_SPEED * 1.15f
        };
        chip.size = {tileSizePx * 0.18f, tileSizePx * 0.14f};
        chip.rotationDeg = angle * 180.0f / PI_CONST;
        chip.angularSpeedDeg = rotDirDist(m_rng) * VfxStyle::WALL_CRACK_ROTATION_SPEED * 1.3f;
        chip.elapsed = 0.0f;
        chip.duration = VfxStyle::WALL_CRUMBLE_DURATION;
        chip.baseColor = VfxStyle::WALL_PIECE_MAIN_COLOR;
        m_brickPieces.push_back(chip);
    }
}

// =============================================================================
// 2. HIỆU ỨNG ĐƯỜNG ĐẠN NÉT ĐỨT TỪ ĐẦU RẮN ĐẾN VIÊN ĐẠN (MÀN 2 - BOSS)
// =============================================================================
void EffectManager::spawnDashedBulletShot(
    sf::Vector2f snakeHeadWorldPos,
    sf::Vector2f targetWorldPos,
    bool isHitBoss
) {
    DashedBulletEffect shot;
    shot.originPos = snakeHeadWorldPos;
    shot.targetPos = targetWorldPos;
    shot.currentBulletPos = snakeHeadWorldPos;
    shot.speed = VfxStyle::BULLET_PROJECTILE_SPEED;
    shot.reachedTarget = false;
    shot.isHit = isHitBoss;
    shot.fadeElapsed = 0.0f;
    shot.fadeDuration = VfxStyle::BULLET_TRAIL_FADE_DURATION;
    m_bullets.push_back(shot);

    // Tạo hiệu ứng tia lửa nhỏ tại điểm viên đạn chạm đích (trúng Boss sáng mạnh hơn trượt)
    const int impactSparks = isHitBoss ? 12 : 5;
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * PI_CONST);
    std::uniform_real_distribution<float> speedDist(40.0f, isHitBoss ? 145.0f : 75.0f);
    for (int i = 0; i < impactSparks; ++i) {
        const float angle = angleDist(m_rng);
        const float spd = speedDist(m_rng);
        SparkParticleEffect spark;
        spark.position = targetWorldPos;
        spark.velocity = {std::cos(angle) * spd, std::sin(angle) * spd};
        spark.radius = isHitBoss ? 3.5f : 2.2f;
        spark.drag = 4.0f;
        spark.elapsed = 0.0f;
        spark.duration = 0.28f;
        spark.baseColor = isHitBoss ? VfxStyle::DASH_TRAIL_HIT_COLOR : VfxStyle::DASH_TRAIL_MISS_COLOR;
        m_sparks.push_back(spark);
    }
}

// =============================================================================
// 3. HIỆU ỨNG SỐ SÁT THƯƠNG BAY LÊN ("-15" HOẶC "MISS")
// =============================================================================
void EffectManager::spawnFloatingDamageText(
    sf::Vector2f worldPos,
    int damageAmount,
    bool isHitBoss
) {
    FloatingTextEffect txt;
    txt.position = {worldPos.x, worldPos.y - 12.0f};
    txt.riseSpeed = VfxStyle::DAMAGE_TEXT_RISE_SPEED;
    txt.elapsed = 0.0f;
    txt.duration = VfxStyle::DAMAGE_TEXT_LIFETIME;

    if (isHitBoss) {
        txt.label = "-" + std::to_string(damageAmount) + " HP";
        txt.fontSize = VfxStyle::DAMAGE_FONT_SIZE_HIT;
        txt.fillColor = VfxStyle::DAMAGE_TEXT_HIT_FILL;
        txt.outlineColor = VfxStyle::DAMAGE_TEXT_HIT_OUTLINE;
    } else {
        txt.label = "MISS";
        txt.fontSize = VfxStyle::DAMAGE_FONT_SIZE_MISS;
        txt.fillColor = VfxStyle::DAMAGE_TEXT_MISS_FILL;
        txt.outlineColor = sf::Color(20, 25, 35, 200);
    }
    m_floatingTexts.push_back(txt);
}

// =============================================================================
// 4. HIỆU ỨNG MÀU MÀN HÌNH KHI SLOW MOTION (CHUYỂN MƯỢT FADE-IN / FADE-OUT)
// =============================================================================
void EffectManager::setSlowMotionVisualActive(bool active) {
    m_slowMoTargetActive = active;
}

// =============================================================================
// 5. HIỆU ỨNG VỤ NỔ KHI QUẢ CẦU BOSS (2x2 Ô) BỊ TIÊU DIỆT
// =============================================================================
void EffectManager::spawnBossDeathExplosion(sf::Vector2f bossCenterWorldPos, float bossDiameterPx) {
    // [CHẾ ĐỘ ẢNH SPRITE SHEET]: Nếu có file Assets/VFX/boss_explosion.png thì hiển thị kèm hoạt ảnh
    if (m_hasBossExplosionSheet &&
        VfxStyle::BOSS_EXPLOSION_SPRITE_COLS > 0 &&
        VfxStyle::BOSS_EXPLOSION_SPRITE_ROWS > 0) {
        SpriteStripEffect anim;
        anim.texture = &m_bossExplosionSheet;
        anim.center = bossCenterWorldPos;
        anim.targetSizePx = bossDiameterPx * 2.2f;
        anim.sheetCols = VfxStyle::BOSS_EXPLOSION_SPRITE_COLS;
        anim.sheetRows = VfxStyle::BOSS_EXPLOSION_SPRITE_ROWS;
        anim.elapsed = 0.0f;
        anim.duration = VfxStyle::BOSS_EXPLOSION_TOTAL_DURATION;
        m_spriteAnims.push_back(anim);
    }

    // Tạo nhiều vòng sóng xung kích nở liên hoàn từ tâm quả cầu Boss 2x2
    for (int i = 0; i < VfxStyle::BOSS_EXPLOSION_RING_COUNT; ++i) {
        ShockwaveRingEffect wave;
        wave.center = bossCenterWorldPos;
        wave.startRadius = bossDiameterPx * 0.20f;
        wave.endRadius = bossDiameterPx * (1.35f + 0.45f * static_cast<float>(i));
        wave.delay = static_cast<float>(i) * 0.11f; // Lệch pha để nổ nhiều đợt liên tiếp
        wave.elapsed = 0.0f;
        wave.duration = 0.65f;
        wave.outlineThickness = 5.0f - static_cast<float>(i) * 0.8f;
        wave.outlineColor = (i % 2 == 0)
                                ? VfxStyle::BOSS_SPARK_PRIMARY_COLOR
                                : VfxStyle::BOSS_SPARK_SECONDARY_COLOR;
        wave.fillColor = (i == 0) ? VfxStyle::BOSS_CORE_FLASH_COLOR : sf::Color(255, 140, 50, 28);
        m_shockwaves.push_back(wave);
    }

    // Sinh mưa hạt năng lượng của Boss bắn tỏa tròn 360 độ rồi lụi dần
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * PI_CONST);
    std::uniform_real_distribution<float> speedDist(
        VfxStyle::BOSS_SPARK_MIN_SPEED,
        VfxStyle::BOSS_SPARK_MAX_SPEED
    );
    std::uniform_real_distribution<float> radiusDist(3.0f, 7.5f);
    std::uniform_real_distribution<float> lifeDist(0.55f, VfxStyle::BOSS_EXPLOSION_TOTAL_DURATION);

    for (int i = 0; i < VfxStyle::BOSS_EXPLOSION_SPARK_COUNT; ++i) {
        const float angle = angleDist(m_rng);
        const float spd = speedDist(m_rng);
        SparkParticleEffect spark;
        spark.position = bossCenterWorldPos;
        spark.velocity = {std::cos(angle) * spd, std::sin(angle) * spd};
        spark.radius = radiusDist(m_rng);
        spark.drag = 2.4f;
        spark.elapsed = 0.0f;
        spark.duration = lifeDist(m_rng);
        spark.baseColor = (i % 3 == 0)
                              ? VfxStyle::BOSS_CORE_FLASH_COLOR
                              : ((i % 2 == 0) ? VfxStyle::BOSS_SPARK_PRIMARY_COLOR
                                              : VfxStyle::BOSS_SPARK_SECONDARY_COLOR);
        m_sparks.push_back(spark);
    }
}

// =============================================================================
// 6. HIỆU ỨNG HẠT NHỎ KHI RẮN VA PHẢI TƯỜNG BỊ DIE (TÙY CHỌN)
// =============================================================================
void EffectManager::spawnSnakeDeathBurst(sf::Vector2f snakeHeadWorldPos) {
    if (!VfxStyle::ENABLE_SNAKE_DEATH_BURST) {
        return;
    }
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * PI_CONST);
    std::uniform_real_distribution<float> speedDist(45.0f, 135.0f);
    for (int i = 0; i < 18; ++i) {
        const float angle = angleDist(m_rng);
        const float spd = speedDist(m_rng);
        SparkParticleEffect spark;
        spark.position = snakeHeadWorldPos;
        spark.velocity = {std::cos(angle) * spd, std::sin(angle) * spd};
        spark.radius = 4.0f;
        spark.drag = 3.2f;
        spark.elapsed = 0.0f;
        spark.duration = 0.55f;
        spark.baseColor = VfxStyle::SNAKE_DEATH_PARTICLE_COLOR;
        m_sparks.push_back(spark);
    }
}

// =============================================================================
// CẬP NHẬT CHUYỂN ĐỘNG & ĐỘ MỜ ALPHA THEO THỜI GIAN THỰC
// =============================================================================
void EffectManager::update(float unscaledDeltaTime) {
    // 1. Làm mờ mượt lớp phủ màn hình Slow Motion (Fade-In / Fade-Out)
    if (m_slowMoTargetActive) {
        m_slowMoBlend = std::min(1.0f, m_slowMoBlend + VfxStyle::SLOW_MO_FADE_IN_RATE * unscaledDeltaTime);
    } else {
        m_slowMoBlend = std::max(0.0f, m_slowMoBlend - VfxStyle::SLOW_MO_FADE_OUT_RATE * unscaledDeltaTime);
    }

    // 2. Cập nhật mảnh gạch vỡ (mờ dần và xóa đúng khi đạt 0.5s)
    for (auto& piece : m_brickPieces) {
        piece.elapsed += unscaledDeltaTime;
        piece.position += piece.velocity * unscaledDeltaTime;
        // Giảm nhẹ vận tốc để mảnh gạch chỉ tách nhẹ tại chỗ
        piece.velocity *= std::max(0.0f, 1.0f - 4.5f * unscaledDeltaTime);
        piece.rotationDeg += piece.angularSpeedDeg * unscaledDeltaTime;
    }
    m_brickPieces.erase(
        std::remove_if(m_brickPieces.begin(), m_brickPieces.end(),
                       [](const BrickPieceEffect& p) { return p.elapsed >= p.duration; }),
        m_brickPieces.end()
    );

    // 3. Cập nhật các vòng sóng xung kích
    for (auto& ring : m_shockwaves) {
        if (ring.delay > 0.0f) {
            ring.delay -= unscaledDeltaTime;
        } else {
            ring.elapsed += unscaledDeltaTime;
        }
    }
    m_shockwaves.erase(
        std::remove_if(m_shockwaves.begin(), m_shockwaves.end(),
                       [](const ShockwaveRingEffect& r) { return r.delay <= 0.0f && r.elapsed >= r.duration; }),
        m_shockwaves.end()
    );

    // 3b. Cập nhật khung vuông chớp nổ 3x3
    for (auto& box : m_squarePulses) {
        box.elapsed += unscaledDeltaTime;
    }
    m_squarePulses.erase(
        std::remove_if(m_squarePulses.begin(), m_squarePulses.end(),
                       [](const SquarePulseEffect& b) { return b.elapsed >= b.duration; }),
        m_squarePulses.end()
    );

    // 4. Cập nhật hạt sáng nổ Boss / tia lửa va chạm
    for (auto& spark : m_sparks) {
        spark.elapsed += unscaledDeltaTime;
        spark.position += spark.velocity * unscaledDeltaTime;
        spark.velocity *= std::max(0.0f, 1.0f - spark.drag * unscaledDeltaTime);
    }
    m_sparks.erase(
        std::remove_if(m_sparks.begin(), m_sparks.end(),
                       [](const SparkParticleEffect& s) { return s.elapsed >= s.duration; }),
        m_sparks.end()
    );

    // 5. Cập nhật viên đạn bay & đường nét đứt từ đầu rắn đến viên đạn
    for (auto& shot : m_bullets) {
        if (!shot.reachedTarget) {
            const sf::Vector2f toTarget = shot.targetPos - shot.currentBulletPos;
            const float remainingDist = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
            const float stepDist = shot.speed * unscaledDeltaTime;

            if (remainingDist <= stepDist || remainingDist <= 0.001f) {
                shot.currentBulletPos = shot.targetPos;
                shot.reachedTarget = true;
            } else {
                shot.currentBulletPos += (toTarget / remainingDist) * stepDist;
            }
        } else {
            shot.fadeElapsed += unscaledDeltaTime;
        }
    }
    m_bullets.erase(
        std::remove_if(m_bullets.begin(), m_bullets.end(),
                       [](const DashedBulletEffect& b) {
                           return b.reachedTarget && b.fadeElapsed >= b.fadeDuration;
                       }),
        m_bullets.end()
    );

    // 6. Cập nhật số sát thương bay lên
    for (auto& txt : m_floatingTexts) {
        txt.elapsed += unscaledDeltaTime;
        txt.position.y -= txt.riseSpeed * unscaledDeltaTime;
    }
    m_floatingTexts.erase(
        std::remove_if(m_floatingTexts.begin(), m_floatingTexts.end(),
                       [](const FloatingTextEffect& t) { return t.elapsed >= t.duration; }),
        m_floatingTexts.end()
    );

    // 7. Cập nhật hoạt ảnh Sprite Sheet PNG (nếu có)
    for (auto& anim : m_spriteAnims) {
        anim.elapsed += unscaledDeltaTime;
    }
    m_spriteAnims.erase(
        std::remove_if(m_spriteAnims.begin(), m_spriteAnims.end(),
                       [](const SpriteStripEffect& a) { return a.elapsed >= a.duration; }),
        m_spriteAnims.end()
    );
}

// =============================================================================
// VẼ TOÀN BỘ HIỆU ỨNG TRONG KHÔNG GIAN BẢN ĐỒ
// =============================================================================
void EffectManager::renderWorldEffects(sf::RenderTarget& target) const {
    // 1. Vẽ vòng sóng xung kích
    for (const auto& ring : m_shockwaves) {
        if (ring.delay > 0.0f) {
            continue;
        }
        const float progress = std::clamp(ring.elapsed / std::max(0.001f, ring.duration), 0.0f, 1.0f);
        const float alphaFactor = 1.0f - progress;
        const float currentRadius = ring.startRadius + (ring.endRadius - ring.startRadius) * progress;

        sf::CircleShape circle(currentRadius, 36);
        circle.setOrigin({currentRadius, currentRadius});
        circle.setPosition(ring.center);
        circle.setFillColor(scaleColorAlpha(ring.fillColor, alphaFactor));
        circle.setOutlineThickness(ring.outlineThickness);
        circle.setOutlineColor(scaleColorAlpha(ring.outlineColor, alphaFactor));
        target.draw(circle);
    }

    // 1b. Vẽ khung vuông chớp sáng đúng chuẩn vùng 3x3 ô quanh đầu rắn khi bấm F
    for (const auto& box : m_squarePulses) {
        const float progress = std::clamp(box.elapsed / std::max(0.001f, box.duration), 0.0f, 1.0f);
        const float alphaFactor = 1.0f - progress;
        const float curSize = box.startSizePx + (box.endSizePx - box.startSizePx) * std::min(1.0f, progress * 2.2f);

        sf::RectangleShape pulseRect({curSize, curSize});
        pulseRect.setOrigin({curSize * 0.5f, curSize * 0.5f});
        pulseRect.setPosition(box.center);
        pulseRect.setFillColor(scaleColorAlpha(box.fillColor, alphaFactor));
        pulseRect.setOutlineThickness(3.0f);
        pulseRect.setOutlineColor(scaleColorAlpha(box.outlineColor, alphaFactor));
        target.draw(pulseRect);
    }

    // 2. Vẽ các mảnh gạch vỡ đang mờ dần (0.5 giây)
    for (const auto& piece : m_brickPieces) {
        const float progress = std::clamp(piece.elapsed / std::max(0.001f, piece.duration), 0.0f, 1.0f);
        const float fadeAlpha = 1.0f - progress;
        const float shrinkScale = 1.0f - 0.22f * progress; // Nhỏ dần nhẹ khi tan biến

        sf::RectangleShape brickShape(piece.size * shrinkScale);
        brickShape.setOrigin((piece.size * shrinkScale) * 0.5f);
        brickShape.setPosition(piece.position);
        brickShape.setRotation(sf::degrees(piece.rotationDeg));
        brickShape.setFillColor(scaleColorAlpha(piece.baseColor, fadeAlpha));
        brickShape.setOutlineThickness(1.0f);
        brickShape.setOutlineColor(scaleColorAlpha(sf::Color(90, 42, 20, 220), fadeAlpha));
        target.draw(brickShape);
    }

    // 3. Vẽ đường đạn nét đứt nối từ đầu rắn đến viên đạn
    for (const auto& shot : m_bullets) {
        const float alphaFactor = shot.reachedTarget
                                      ? (1.0f - std::clamp(shot.fadeElapsed / std::max(0.001f, shot.fadeDuration), 0.0f, 1.0f))
                                      : 1.0f;
        const sf::Color baseDashColor = shot.isHit
                                            ? VfxStyle::DASH_TRAIL_HIT_COLOR
                                            : VfxStyle::DASH_TRAIL_MISS_COLOR;

        // Vẽ đường nét đứt từ vị trí đầu rắn -> vị trí hiện tại của viên đạn
        drawDashedLine(
            target,
            shot.originPos,
            shot.currentBulletPos,
            VfxStyle::BULLET_DASH_LENGTH,
            VfxStyle::BULLET_DASH_GAP,
            VfxStyle::BULLET_DASH_THICKNESS,
            scaleColorAlpha(baseDashColor, alphaFactor)
        );

        // Vẽ đầu viên đạn ở mũi đường nét đứt
        if (m_hasBulletTexture) {
            sf::Sprite bulletSprite(m_bulletTexture);
            const sf::Vector2u texSize = m_bulletTexture.getSize();
            bulletSprite.setOrigin({texSize.x * 0.5f, texSize.y * 0.5f});
            bulletSprite.setPosition(shot.currentBulletPos);
            bulletSprite.setColor(scaleColorAlpha(sf::Color::White, alphaFactor));
            target.draw(bulletSprite);
        } else {
            sf::CircleShape bulletHead(VfxStyle::BULLET_HEAD_RADIUS, 16);
            bulletHead.setOrigin({VfxStyle::BULLET_HEAD_RADIUS, VfxStyle::BULLET_HEAD_RADIUS});
            bulletHead.setPosition(shot.currentBulletPos);
            bulletHead.setFillColor(scaleColorAlpha(VfxStyle::BULLET_HEAD_COLOR, alphaFactor));
            bulletHead.setOutlineThickness(2.0f);
            bulletHead.setOutlineColor(scaleColorAlpha(baseDashColor, alphaFactor));
            target.draw(bulletHead);
        }
    }

    // 4. Vẽ hạt tia lửa / hạt nổ Boss
    for (const auto& spark : m_sparks) {
        const float progress = std::clamp(spark.elapsed / std::max(0.001f, spark.duration), 0.0f, 1.0f);
        const float alphaFactor = 1.0f - progress;
        const float currentRadius = std::max(1.0f, spark.radius * (1.0f - 0.45f * progress));

        sf::CircleShape dot(currentRadius, 12);
        dot.setOrigin({currentRadius, currentRadius});
        dot.setPosition(spark.position);
        dot.setFillColor(scaleColorAlpha(spark.baseColor, alphaFactor));
        target.draw(dot);
    }

    // 5. Vẽ các hoạt ảnh Sprite Sheet tùy chọn (hỗ trợ cả dải ngang 1 hàng & lưới nhiều hàng)
    for (const auto& anim : m_spriteAnims) {
        const int totalFrames = anim.sheetCols * anim.sheetRows;
        if (!anim.texture || totalFrames <= 0) {
            continue;
        }
        const float progress = std::clamp(anim.elapsed / std::max(0.001f, anim.duration), 0.0f, 0.999f);
        const int frameIndex = static_cast<int>(progress * static_cast<float>(totalFrames));
        const int colIdx = frameIndex % anim.sheetCols;
        const int rowIdx = frameIndex / anim.sheetCols;

        const sf::Vector2u texSize = anim.texture->getSize();
        const int frameW = static_cast<int>(texSize.x) / anim.sheetCols;
        const int frameH = static_cast<int>(texSize.y) / anim.sheetRows;
        if (frameW <= 0 || frameH <= 0) {
            continue;
        }

        sf::Sprite sprite(
            *anim.texture,
            sf::IntRect({colIdx * frameW, rowIdx * frameH}, {frameW, frameH})
        );
        sprite.setOrigin({frameW * 0.5f, frameH * 0.5f});
        const float scale = anim.targetSizePx / static_cast<float>(std::max(frameW, frameH));
        sprite.setScale({scale, scale});
        sprite.setPosition(anim.center);
        target.draw(sprite);
    }

    // 6. Vẽ chữ số sát thương ("-15 HP" / "MISS") nổi lên trên cùng
    if (const sf::Font* font = resolveActiveFont()) {
        for (const auto& item : m_floatingTexts) {
            const float progress = std::clamp(item.elapsed / std::max(0.001f, item.duration), 0.0f, 1.0f);
            const float alphaFactor = 1.0f - progress * progress;

            sf::Text text(*font, item.label, item.fontSize);
            text.setStyle(sf::Text::Bold);
            text.setFillColor(scaleColorAlpha(item.fillColor, alphaFactor));
            text.setOutlineThickness(2.0f);
            text.setOutlineColor(scaleColorAlpha(item.outlineColor, alphaFactor));

            const sf::FloatRect bounds = text.getLocalBounds();
            text.setOrigin({bounds.position.x + bounds.size.x * 0.5f,
                            bounds.position.y + bounds.size.y * 0.5f});
            text.setPosition(item.position);
            target.draw(text);
        }
    }
}

// =============================================================================
// VẼ LỚP PHỦ MÀN HÌNH SLOW MOTION (MỜ DẦN MƯỢT MÀ BẰNG SMOOTHSTEP)
// =============================================================================
void EffectManager::renderScreenOverlay(sf::RenderTarget& target, sf::Vector2f screenSize) const {
    if (m_slowMoBlend <= 0.001f) {
        return;
    }

    // Hàm nội suy bậc ba Smoothstep giúp lúc bắt đầu hiện và lúc tắt hẳn cực êm
    const float smoothFactor = m_slowMoBlend * m_slowMoBlend * (3.0f - 2.0f * m_slowMoBlend);

    // 1. Tấm phủ màu lạnh toàn màn hình
    sf::RectangleShape overlay(screenSize);
    overlay.setPosition({0.0f, 0.0f});
    overlay.setFillColor(scaleColorAlpha(VfxStyle::SLOW_MO_SCREEN_TINT, smoothFactor));
    target.draw(overlay);

    // 2. Viền sáng nhẹ quanh mép cửa sổ báo hiệu trạng thái ngưng đọng thời gian
    const float inset = 4.0f;
    sf::RectangleShape borderFrame({screenSize.x - inset * 2.0f, screenSize.y - inset * 2.0f});
    borderFrame.setPosition({inset, inset});
    borderFrame.setFillColor(sf::Color::Transparent);
    borderFrame.setOutlineThickness(inset);
    borderFrame.setOutlineColor(scaleColorAlpha(VfxStyle::SLOW_MO_BORDER_GLOW, smoothFactor));
    target.draw(borderFrame);
}

// =============================================================================
// HÀM TIỆN ÍCH VẼ ĐƯỜNG NÉT ĐỨT (DASHED LINE) GIỮA 2 ĐIỂM BẤT KỲ
// =============================================================================
void EffectManager::drawDashedLine(
    sf::RenderTarget& target,
    sf::Vector2f startPos,
    sf::Vector2f endPos,
    float dashLength,
    float gapLength,
    float thickness,
    sf::Color color
) {
    const sf::Vector2f delta = endPos - startPos;
    const float totalLength = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (totalLength <= 0.5f || dashLength <= 0.5f) {
        return;
    }

    const sf::Vector2f unitDir = delta / totalLength;
    const float angleRadians = std::atan2(delta.y, delta.x);
    const float stride = dashLength + std::max(1.0f, gapLength);

    for (float dist = 0.0f; dist < totalLength; dist += stride) {
        const float currentSegLen = std::min(dashLength, totalLength - dist);
        const sf::Vector2f segStart = startPos + unitDir * dist;

        sf::RectangleShape segShape({currentSegLen, thickness});
        segShape.setOrigin({0.0f, thickness * 0.5f});
        segShape.setPosition(segStart);
        segShape.setRotation(sf::radians(angleRadians));
        segShape.setFillColor(color);
        target.draw(segShape);
    }
}

std::size_t EffectManager::getActiveWorldEffectCount() const {
    return m_brickPieces.size() + m_shockwaves.size() + m_squarePulses.size() + m_sparks.size() +
           m_bullets.size() + m_floatingTexts.size() + m_spriteAnims.size();
}

float EffectManager::getSlowMotionOverlayBlend() const {
    return m_slowMoBlend;
}

const sf::Font* EffectManager::resolveActiveFont() const {
    if (m_externalFont) {
        return m_externalFont;
    }
    if (m_hasOwnedFont) {
        return &m_ownedFont;
    }
    return nullptr;
}

sf::Color EffectManager::scaleColorAlpha(sf::Color base, float factor01) {
    const float clamped = std::clamp(factor01, 0.0f, 1.0f);
    base.a = static_cast<std::uint8_t>(std::round(static_cast<float>(base.a) * clamped));
    return base;
}
