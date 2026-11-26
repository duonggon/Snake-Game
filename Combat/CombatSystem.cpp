#include "Combat/CombatSystem.h"
#include "Core/Config.h"
#include "Effects/EffectManager.h"
#include "Effects/ScreenShake.h"
#include "Effects/VfxStyleConfig.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <algorithm>
#include <cmath>

CombatSystem::CombatSystem()
    : m_currentStage(GameStage::Stage1_Survival),
      m_aimState(AimState::Ready),
      m_aimTimeRemaining(0.0f),
      m_cooldownRemaining(0.0f),
      m_stats{},
      m_frameEvents{} {}

void CombatSystem::resetAll(GameStage initialStage) {
    m_currentStage = initialStage;
    m_aimState = AimState::Ready;
    m_aimTimeRemaining = 0.0f;
    m_cooldownRemaining = 0.0f;
    m_stats = CombatStats{};
    m_frameEvents = CombatEventFlags{};
}

void CombatSystem::setGameStage(GameStage stage, EffectManager* effects) {
    m_currentStage = stage;
    // Nếu quay về Màn 1 Sinh tồn thì tắt ngay lập tức chế độ ngắm súng & Slow Motion
    if (m_currentStage == GameStage::Stage1_Survival && m_aimState != AimState::Ready) {
        m_aimState = AimState::Ready;
        m_aimTimeRemaining = 0.0f;
        m_cooldownRemaining = 0.0f;
        if (effects) {
            effects->setSlowMotionVisualActive(false);
        }
    }
}

GameStage CombatSystem::getGameStage() const {
    return m_currentStage;
}

// =============================================================================
// 1. KỸ NĂNG PHÁ TƯỜNG (PHÍM F - TRỪ 1 ĐỐT RẮN, CẦN ĐỘ DÀI > 3 ĐỐT)
// =============================================================================
bool CombatSystem::canUseWallBreak(const ICombatPlayer& player) const {
    if (!player.isAlive()) {
        return false;
    }
    // Khi rắn ở độ dài tối thiểu <= 3 đốt thì KHÔNG THỂ phá tường được nữa
    return player.getSegmentCount() > Config::MIN_SNAKE_LENGTH_FOR_WALL_BREAK;
}

int CombatSystem::tryActivateWallBreak(
    ICombatMap& map,
    ICombatPlayer& player,
    EffectManager& effects,
    ScreenShake& screenShake
) {
    if (!canUseWallBreak(player)) {
        return -1;
    }

    // 1. Trừ ngay 1 đốt rắn (kể cả xung quanh có tường phá được hay không)
    player.shrinkSegments(Config::WALL_BREAK_SEGMENT_COST);

    // 2. Xác định tâm đầu rắn và chớp sáng khung vuông chuẩn 3x3 ô (3 * TILE_SIZE)
    const sf::Vector2i headPos = player.getHeadGridPos();
    const sf::Vector2f headWorld = map.gridToWorldCenter(headPos.x, headPos.y);
    const float blastBoxWidthPx =
        static_cast<float>(Config::WALL_BREAK_HALF_EXTENT * 2 + 1) * Config::TILE_SIZE;
    effects.spawnWallSkillPulse(headWorld, blastBoxWidthPx);

    // 3. Duyệt vùng lưới 3 x 3 quanh đầu rắn và chỉ phá các tường hợp lệ (DestructibleWall)
    int destroyedCount = 0;
    const int extent = Config::WALL_BREAK_HALF_EXTENT;
    for (int row = headPos.y - extent; row <= headPos.y + extent; ++row) {
        for (int col = headPos.x - extent; col <= headPos.x + extent; ++col) {
            if (!map.isInsideBounds(col, row)) {
                continue;
            }
            if (map.isDestructibleWall(col, row)) {
                map.destroyWall(col, row);
                // Tạo hiệu ứng gạch nứt vỡ rồi mờ dần biến mất trong 0.5 giây tại đúng ô đó
                effects.spawnWallCrumbleEffect(map.gridToWorldCenter(col, row), Config::TILE_SIZE);
                destroyedCount += 1;
            }
        }
    }

    // 4. Kích hoạt rung màn hình nhẹ & cập nhật thống kê / cờ sự kiện âm thanh
    screenShake.trigger();
    m_stats.wallBreakUsedCount += 1;
    m_stats.totalWallsDestroyed += destroyedCount;
    m_frameEvents.wallBreakActivated = true;

    return destroyedCount;
}

// =============================================================================
// 2. HỆ THỐNG NGẮM BẮN BOSS (CHỈ HOẠT ĐỘNG Ở MÀN 2 - ĐÁNH BOSS)
// =============================================================================
bool CombatSystem::tryToggleAim(
    const ICombatPlayer& player,
    const ICombatBoss* boss,
    EffectManager& effects
) {
    // Ở Màn 1 (Sinh tồn): Rắn không sử dụng súng -> bỏ qua hoàn toàn phím Space
    if (m_currentStage != GameStage::Stage2_BossFight) {
        return false;
    }

    // Nếu đang ở chế độ ngắm -> nhấn Space lần 2 (Toggle OFF) sẽ thoát ngắm & vào hồi chiêu 5s
    if (m_aimState == AimState::Aiming) {
        enterCooldownState(&effects);
        return true;
    }

    // Nếu đang trong 5 giây hồi chiêu -> không cho bật lại Slow Motion
    if (m_aimState == AimState::Cooldown) {
        return false;
    }

    // Điều kiện bật chế độ ngắm: Rắn còn sống, Boss đang xuất hiện & còn sống, còn ít nhất 1 viên đạn
    if (!player.isAlive() || !boss || !boss->isActive() || !boss->isAlive()) {
        return false;
    }
    if (player.getAmmo() < Config::AMMO_COST_PER_SHOT) {
        return false;
    }

    m_aimState = AimState::Aiming;
    m_aimTimeRemaining = Config::MAX_AIM_DURATION;
    effects.setSlowMotionVisualActive(true);
    return true;
}

CombatSystem::ShotResult CombatSystem::tryFireShot(
    sf::Vector2f mouseWorldPos,
    const ICombatMap& map,
    ICombatPlayer& player,
    ICombatBoss* boss,
    EffectManager& effects,
    ScreenShake& screenShake
) {
    ShotResult result;

    // Chỉ cho phép khai hỏa ở Màn 2 khi đang bật chế độ ngắm (Space) và còn đạn
    if (m_currentStage != GameStage::Stage2_BossFight || m_aimState != AimState::Aiming) {
        return result;
    }
    if (!player.isAlive() || player.getAmmo() < Config::AMMO_COST_PER_SHOT) {
        enterCooldownState(&effects);
        return result;
    }

    // 1. Tiêu hao 1 viên đạn ngay khi bắn (dù bắn trong tầm hay vượt ngoài rìa 5 ô)
    if (!player.consumeAmmo(Config::AMMO_COST_PER_SHOT)) {
        enterCooldownState(&effects);
        return result;
    }

    result.fired = true;
    m_stats.shotsFired += 1;
    m_frameEvents.gunShotFired = true;

    // 2. Tính điểm xuất phát từ tâm đầu rắn & kẹp điểm đích tối đa tại rìa vòng 5 ô
    const sf::Vector2i headGrid = player.getHeadGridPos();
    const sf::Vector2f originWorld = map.gridToWorldCenter(headGrid.x, headGrid.y);
    bool wasClamped = false;
    const sf::Vector2f clampedEnd = clampTargetToRange(
        originWorld,
        mouseWorldPos,
        Config::AIM_RANGE_PIXELS,
        &wasClamped
    );

    result.originWorld = originWorld;
    result.clampedToMaxRange = wasClamped;
    result.impactWorld = clampedEnd;

    // 3. Kiểm tra đường đạn từ đầu rắn tới điểm đích (tối đa 5 ô) có chạm Hitbox 2x2 của Boss không
    if (boss && boss->isActive() && boss->isAlive()) {
        const sf::FloatRect bossHitbox = boss->getWorldHitbox();
        if (const std::optional<float> hitParam =
                intersectSegmentWithRect(originWorld, clampedEnd, bossHitbox)) {
            result.hitBoss = true;
            // Viên đạn dừng ngay tại bề mặt chạm Boss (hoặc tại điểm click nếu nằm trong thân Boss)
            result.impactWorld = originWorld + (clampedEnd - originWorld) * (*hitParam);
        }
    }

    // 4. Phát hiệu ứng đường đạn nét đứt nối từ đầu rắn đến viên đạn
    effects.spawnDashedBulletShot(originWorld, result.impactWorld, result.hitBoss);

    // 5. Xử lý sát thương 15 HP nếu trúng Boss hoặc báo MISS nếu trượt
    if (result.hitBoss && boss) {
        m_stats.shotsHit += 1;
        m_frameEvents.bossHit = true;

        const sf::FloatRect bossRectBeforeHit = boss->getWorldHitbox();
        result.killedBoss = boss->takeDamage(Config::BOSS_SHOT_DAMAGE);

        effects.spawnFloatingDamageText(result.impactWorld, Config::BOSS_SHOT_DAMAGE, true);
        screenShake.trigger();

        // Nếu phát đạn tiêu diệt Boss về 0 HP -> kích hoạt vụ nổ lớn hủy diệt quả cầu Boss 2x2
        if (result.killedBoss) {
            m_frameEvents.bossDefeated = true;
            const sf::Vector2f bossCenter = {
                bossRectBeforeHit.position.x + bossRectBeforeHit.size.x * 0.5f,
                bossRectBeforeHit.position.y + bossRectBeforeHit.size.y * 0.5f
            };
            const float bossDiameterPx = std::max(bossRectBeforeHit.size.x, bossRectBeforeHit.size.y);
            effects.spawnBossDeathExplosion(bossCenter, bossDiameterPx);
            screenShake.trigger(); // Cộng dồn thêm 1 nhịp theo cấp số nhân khi Boss nổ
        }
    } else {
        effects.spawnFloatingDamageText(result.impactWorld, 0, false);
    }

    // 6. Bắn xong đúng 1 viên là tự thoát chế độ ngắm ngay lập tức và vào hồi chiêu 5.0 giây!
    enterCooldownState(&effects);
    return result;
}

void CombatSystem::update(
    float unscaledDeltaTime,
    const ICombatPlayer& player,
    const ICombatBoss* boss,
    EffectManager& effects
) {
    if (m_aimState == AimState::Aiming) {
        // Tự động ngắt ngắm nếu rắn chết, Boss không còn tồn tại, hoặc hết sạch đạn
        const bool invalidContext =
            (m_currentStage != GameStage::Stage2_BossFight) ||
            !player.isAlive() ||
            !boss || !boss->isActive() || !boss->isAlive() ||
            (player.getAmmo() < Config::AMMO_COST_PER_SHOT);

        if (invalidContext) {
            enterCooldownState(&effects);
            return;
        }

        // Giới hạn thời gian giữ ngắm tối đa để tránh người chơi lạm dụng Slow Motion mãi
        m_aimTimeRemaining -= unscaledDeltaTime;
        if (m_aimTimeRemaining <= 0.0f) {
            enterCooldownState(&effects);
        }
    } else if (m_aimState == AimState::Cooldown) {
        m_cooldownRemaining -= unscaledDeltaTime;
        if (m_cooldownRemaining <= 0.0f) {
            m_cooldownRemaining = 0.0f;
            m_aimState = AimState::Ready;
        }
    }
}

void CombatSystem::renderAimingWorldUI(
    sf::RenderTarget& target,
    sf::Vector2f mouseWorldPos,
    const ICombatMap& map,
    const ICombatPlayer& player,
    const ICombatBoss* boss
) const {
    if (m_currentStage != GameStage::Stage2_BossFight || m_aimState != AimState::Aiming || !player.isAlive()) {
        return;
    }

    const sf::Vector2i headGrid = player.getHeadGridPos();
    const sf::Vector2f originWorld = map.gridToWorldCenter(headGrid.x, headGrid.y);

    // 1. Vẽ vòng tròn bán kính ngắm 5 ô quanh tâm đầu rắn
    sf::CircleShape rangeRing(Config::AIM_RANGE_PIXELS, 64);
    rangeRing.setOrigin({Config::AIM_RANGE_PIXELS, Config::AIM_RANGE_PIXELS});
    rangeRing.setPosition(originWorld);
    rangeRing.setFillColor(VfxStyle::AIM_RING_FILL_COLOR);
    rangeRing.setOutlineThickness(VfxStyle::AIM_RING_OUTLINE_THICKNESS);
    rangeRing.setOutlineColor(VfxStyle::AIM_RING_OUTLINE_COLOR);
    target.draw(rangeRing);

    // 2. Tính điểm ngắm giới hạn tại rìa 5 ô và kiểm tra đường ngắm có đang khóa trúng Boss không
    bool wasClamped = false;
    sf::Vector2f previewEnd = clampTargetToRange(
        originWorld,
        mouseWorldPos,
        Config::AIM_RANGE_PIXELS,
        &wasClamped
    );

    bool lockingOnBoss = false;
    if (boss && boss->isActive() && boss->isAlive()) {
        if (const std::optional<float> hitT =
                intersectSegmentWithRect(originWorld, previewEnd, boss->getWorldHitbox())) {
            lockingOnBoss = true;
            previewEnd = originWorld + (previewEnd - originWorld) * (*hitT);
        }
    }

    const sf::Color previewColor = lockingOnBoss
                                       ? VfxStyle::AIM_CROSSHAIR_LOCK_BOSS
                                       : (wasClamped ? VfxStyle::AIM_PREVIEW_DASH_CLAMPED
                                                     : VfxStyle::AIM_PREVIEW_DASH_IN_RANGE);

    // 3. Vẽ đường ngắm nét đứt xem trước từ đầu rắn tới điểm đạn sẽ tới
    EffectManager::drawDashedLine(
        target,
        originWorld,
        previewEnd,
        VfxStyle::BULLET_DASH_LENGTH,
        VfxStyle::BULLET_DASH_GAP,
        2.2f,
        previewColor
    );

    // 4. Vẽ tâm ngắm Crosshair tại vị trí điểm đích thực tế
    const float crossRadius = 8.0f;
    sf::CircleShape crosshairRing(crossRadius, 20);
    crosshairRing.setOrigin({crossRadius, crossRadius});
    crosshairRing.setPosition(previewEnd);
    crosshairRing.setFillColor(sf::Color::Transparent);
    crosshairRing.setOutlineThickness(2.0f);
    crosshairRing.setOutlineColor(previewColor);
    target.draw(crosshairRing);

    sf::RectangleShape hBar({crossRadius * 2.6f, 2.0f});
    hBar.setOrigin({crossRadius * 1.3f, 1.0f});
    hBar.setPosition(previewEnd);
    hBar.setFillColor(previewColor);
    target.draw(hBar);

    sf::RectangleShape vBar({2.0f, crossRadius * 2.6f});
    vBar.setOrigin({1.0f, crossRadius * 1.3f});
    vBar.setPosition(previewEnd);
    vBar.setFillColor(previewColor);
    target.draw(vBar);

    // 5. Vẽ thanh thời gian giữ ngắm còn lại nhỏ gọn ngay dưới đầu rắn
    const float barWidth = Config::TILE_SIZE * 1.35f;
    const float barHeight = 5.0f;
    const float ratio = std::clamp(m_aimTimeRemaining / std::max(0.001f, Config::MAX_AIM_DURATION), 0.0f, 1.0f);

    sf::RectangleShape barBg({barWidth, barHeight});
    barBg.setOrigin({barWidth * 0.5f, barHeight * 0.5f});
    barBg.setPosition({originWorld.x, originWorld.y + Config::TILE_SIZE * 0.70f});
    barBg.setFillColor(sf::Color(15, 20, 30, 190));
    target.draw(barBg);

    sf::RectangleShape barFill({barWidth * ratio, barHeight});
    barFill.setOrigin({0.0f, barHeight * 0.5f});
    barFill.setPosition({originWorld.x - barWidth * 0.5f, originWorld.y + Config::TILE_SIZE * 0.70f});
    barFill.setFillColor(sf::Color(95, 220, 255, 230));
    target.draw(barFill);
}

float CombatSystem::getTimeScale() const {
    if (m_currentStage == GameStage::Stage2_BossFight && m_aimState == AimState::Aiming) {
        return Config::SLOW_MOTION_FACTOR; // 0.30f (30%)
    }
    return Config::NORMAL_TIME_SCALE; // 1.00f (100%)
}

CombatSystem::AimState CombatSystem::getAimState() const {
    return m_aimState;
}

bool CombatSystem::isAiming() const {
    return m_aimState == AimState::Aiming;
}

float CombatSystem::getAimTimeRemaining() const {
    return m_aimTimeRemaining;
}

float CombatSystem::getCooldownRemaining() const {
    return m_cooldownRemaining;
}

const CombatStats& CombatSystem::getStats() const {
    return m_stats;
}

const CombatEventFlags& CombatSystem::getLastFrameEvents() const {
    return m_frameEvents;
}

void CombatSystem::clearFrameEvents() {
    m_frameEvents = CombatEventFlags{};
}

// =============================================================================
// CÁC HÀM TOÁN HỌC ĐƯỜNG ĐẠN: KẸP BÁN KÍNH 5 Ô & GIAO CẮT TIA - HITBOX 2x2
// =============================================================================
sf::Vector2f CombatSystem::clampTargetToRange(
    sf::Vector2f originWorld,
    sf::Vector2f mouseWorld,
    float maxRangePx,
    bool* outWasClamped
) {
    const sf::Vector2f diff = mouseWorld - originWorld;
    const float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

    if (dist <= maxRangePx || dist <= 0.0001f) {
        if (outWasClamped) {
            *outWasClamped = false;
        }
        return mouseWorld;
    }

    if (outWasClamped) {
        *outWasClamped = true;
    }
    return originWorld + (diff / dist) * maxRangePx;
}

std::optional<float> CombatSystem::intersectSegmentWithRect(
    sf::Vector2f segStart,
    sf::Vector2f segEnd,
    const sf::FloatRect& rect
) {
    // Nếu điểm đầu rắn đã nằm chạm ngay bên trong Hitbox 2x2 của Boss
    if (rect.contains(segStart)) {
        return 0.0f;
    }

    // Thuật toán Slab Ray-AABB kiểm tra đoạn thẳng P(t) = segStart + t * dir, với t thuộc [0, 1]
    const sf::Vector2f dir = segEnd - segStart;
    const float minX = rect.position.x;
    const float maxX = rect.position.x + rect.size.x;
    const float minY = rect.position.y;
    const float maxY = rect.position.y + rect.size.y;

    float tMin = 0.0f;
    float tMax = 1.0f;

    // Kiểm tra trục X
    if (std::abs(dir.x) < 1e-6f) {
        if (segStart.x < minX || segStart.x > maxX) {
            return std::nullopt;
        }
    } else {
        const float invDx = 1.0f / dir.x;
        float t1 = (minX - segStart.x) * invDx;
        float t2 = (maxX - segStart.x) * invDx;
        if (t1 > t2) {
            std::swap(t1, t2);
        }
        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);
        if (tMin > tMax) {
            return std::nullopt;
        }
    }

    // Kiểm tra trục Y
    if (std::abs(dir.y) < 1e-6f) {
        if (segStart.y < minY || segStart.y > maxY) {
            return std::nullopt;
        }
    } else {
        const float invDy = 1.0f / dir.y;
        float t1 = (minY - segStart.y) * invDy;
        float t2 = (maxY - segStart.y) * invDy;
        if (t1 > t2) {
            std::swap(t1, t2);
        }
        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);
        if (tMin > tMax) {
            return std::nullopt;
        }
    }

    if (tMin >= 0.0f && tMin <= 1.0f) {
        return tMin;
    }
    return std::nullopt;
}

void CombatSystem::enterCooldownState(EffectManager* effects) {
    m_aimState = AimState::Cooldown;
    m_aimTimeRemaining = 0.0f;
    m_cooldownRemaining = Config::AIM_COOLDOWN_DURATION; // 5.0 giây hồi chiêu
    if (effects) {
        effects->setSlowMotionVisualActive(false);
    }
}
