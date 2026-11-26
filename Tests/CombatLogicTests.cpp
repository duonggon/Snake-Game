#include "Combat/CombatInterfaces.h"
#include "Combat/CombatSystem.h"
#include "Core/Config.h"
#include "Core/SpawnRules.h"
#include "Effects/EffectManager.h"
#include "Effects/ScreenShake.h"
#include "Effects/VfxStyleConfig.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

// =============================================================================
// BỘ KIỂM THỬ LOGIC TỰ ĐỘNG (Tests/CombatLogicTests.cpp)
// Kiểm chứng toàn bộ các điều kiện biên và công thức toán học của Huỳnh Nhựt Cường:
// 1. Màn 1 khóa hoàn toàn hệ thống súng & Slow Motion.
// 2. Kỹ năng phá tường Phím F: khóa khi rắn <= 3 đốt; nổ trừ 1 đốt khi >= 4 đốt
//    kể cả không có tường; chỉ phá DestructibleWall trong 3x3, giữ FixedWall.
// 3. Hiệu ứng gạch vỡ biến mất đúng sau 0.5 giây.
// 4. Màn 2 Ngắm bắn: Space Toggle bật Slow-Mo 30%, bắn 1 phát trừ 1 đạn + thoát ngay
//    + hồi chiêu đúng 5.0 giây.
// 5. Click ngoài bán kính 5 ô: vẫn tốn 1 đạn, đường đạn dừng đúng ở rìa 5 ô.
// 6. Hạ Boss 100 HP sau 7 phát trúng (15 HP/phát) kích hoạt hiệu ứng nổ Boss.
// 7. Công thức rung màn hình: giới hạn trần 6/10 & tổng thời gian cấp số nhân (u1=1.5s, q=0.3).
// =============================================================================
namespace {

int g_failedAssertions = 0;

void expectTrue(bool condition, const char* description) {
    if (!condition) {
        std::cerr << "[THAT BAI] " << description << "\n";
        g_failedAssertions += 1;
    } else {
        std::cout << "[DAT] " << description << "\n";
    }
}

bool approxEqual(float a, float b, float eps = 1e-3f) {
    return std::abs(a - b) <= eps;
}

class MockMap final : public ICombatMap {
public:
    enum Cell { Empty, Fixed, Destructible };
    std::vector<Cell> grid = std::vector<Cell>(Config::MAP_COLS * Config::MAP_ROWS, Empty);

    void set(int c, int r, Cell val) { grid[static_cast<std::size_t>(r * Config::MAP_COLS + c)] = val; }
    Cell get(int c, int r) const { return grid[static_cast<std::size_t>(r * Config::MAP_COLS + c)]; }

    bool isInsideBounds(int col, int row) const override {
        return col >= 0 && col < Config::MAP_COLS && row >= 0 && row < Config::MAP_ROWS;
    }
    bool isDestructibleWall(int col, int row) const override {
        return isInsideBounds(col, row) && get(col, row) == Destructible;
    }
    void destroyWall(int col, int row) override {
        if (isDestructibleWall(col, row)) {
            set(col, row, Empty);
        }
    }
    sf::Vector2f gridToWorldCenter(int col, int row) const override {
        return {(col + 0.5f) * Config::TILE_SIZE, (row + 0.5f) * Config::TILE_SIZE};
    }
};

class MockPlayer final : public ICombatPlayer {
public:
    bool alive = true;
    sf::Vector2i head{5, 5};
    int segments = 5;
    int ammo = 10;

    bool isAlive() const override { return alive; }
    sf::Vector2i getHeadGridPos() const override { return head; }
    int getSegmentCount() const override { return segments; }
    void shrinkSegments(int count) override { segments = std::max(1, segments - count); }
    int getAmmo() const override { return ammo; }
    bool consumeAmmo(int count) override {
        if (ammo < count) return false;
        ammo -= count;
        return true;
    }
};

class MockBoss final : public ICombatBoss {
public:
    bool active = true;
    int hp = Config::BOSS_MAX_HP;
    sf::FloatRect box{{8.0f * Config::TILE_SIZE, 4.0f * Config::TILE_SIZE},
                      {2.0f * Config::TILE_SIZE, 2.0f * Config::TILE_SIZE}};

    bool isActive() const override { return active; }
    bool isAlive() const override { return hp > 0; }
    sf::FloatRect getWorldHitbox() const override { return box; }
    bool takeDamage(int damageAmount) override {
        hp = std::max(0, hp - damageAmount);
        return hp == 0;
    }
};

} // namespace

int main() {
    // -------------------------------------------------------------------------
    // KIỂM THỬ 1: Màn 1 khóa hoàn toàn súng & Slow Motion
    // -------------------------------------------------------------------------
    {
        MockMap map;
        MockPlayer player;
        MockBoss boss;
        EffectManager effects;
        ScreenShake shake;
        CombatSystem combat;
        combat.resetAll(GameStage::Stage1_Survival);

        expectTrue(!combat.tryToggleAim(player, &boss, effects),
                   "Man 1: Nhan Space khong bat che do ngam ban");
        expectTrue(approxEqual(combat.getTimeScale(), 1.0f),
                   "Man 1: TimeScale luon giu nguyen 100%");
        const auto shot = combat.tryFireShot({300.f, 200.f}, map, player, &boss, effects, shake);
        expectTrue(!shot.fired && player.ammo == 10,
                   "Man 1: Click chuot khong ban dan va khong tru dan");
    }

    // -------------------------------------------------------------------------
    // KIỂM THỬ 2: Phá tường Phím F (ngưỡng 3 đốt, nổ vùng trống vẫn trừ 1 đốt, giữ FixedWall)
    // -------------------------------------------------------------------------
    {
        MockMap map;
        MockPlayer player;
        EffectManager effects;
        ScreenShake shake;
        CombatSystem combat;

        // Rắn có 4 đốt đứng giữa vùng trống -> nổ thành công, giảm còn 3 đốt
        player.segments = 4;
        player.head = {10, 10};
        const int emptyBlast = combat.tryActivateWallBreak(map, player, effects, shake);
        expectTrue(emptyBlast == 0 && player.segments == 3,
                   "Phim F: Ran 4 dot nhan F giua vung trong van no va tru con 3 dot");

        // Rắn còn đúng 3 đốt -> khóa kỹ năng phá tường
        const int blockedBlast = combat.tryActivateWallBreak(map, player, effects, shake);
        expectTrue(blockedBlast == -1 && player.segments == 3,
                   "Phim F: Ran co do dai toi thieu 3 dot thi khong the pha tuong nua");

        // Rắn 5 đốt đứng ở góc (0, 0) cạnh cả tường cố định và tường phá được
        player.segments = 5;
        player.head = {0, 0};
        map.set(0, 1, MockMap::Fixed);
        map.set(1, 0, MockMap::Destructible);
        map.set(1, 1, MockMap::Destructible);
        map.set(2, 2, MockMap::Destructible); // Ngoài vùng 3x3 quanh (0,0)

        const int cornerBlast = combat.tryActivateWallBreak(map, player, effects, shake);
        expectTrue(cornerBlast == 2 && player.segments == 4,
                   "Phim F: Pha dung 2 tuong hop le trong vung 3x3 o goc ban do va tru 1 dot");
        expectTrue(map.get(0, 1) == MockMap::Fixed && map.get(2, 2) == MockMap::Destructible,
                   "Phim F: Giu nguyen FixedWall va tuong nam ngoai pham vi 3x3");

        // Đặt 8 bức tường bao vây kín cả 8 ô xung quanh đầu rắn (5, 5) trong phạm vi 3x3:
        // Chỉ 1 lần nhấn phím F duy nhất phải phá hủy ĐỒNG LOẠT cả 8 bức tường!
        player.segments = 6;
        player.head = {5, 5};
        for (int r = 4; r <= 6; ++r) {
            for (int c = 4; c <= 6; ++c) {
                if (c != 5 || r != 5) {
                    map.set(c, r, MockMap::Destructible);
                }
            }
        }
        const int fullSurroundBlast = combat.tryActivateWallBreak(map, player, effects, shake);
        expectTrue(fullSurroundBlast == 8 && player.segments == 5,
                   "Phim F: Pha huy DONG LOAT tat ca 8/8 buc tuong trong pham vi 3x3 chi voi 1 lan nhan F");

        // Kiểm chứng bấm nổ tường LIÊN TỤC 5 lần liền nhau trong 0.000 giây (0s hồi chiêu!)
        player.segments = 8;
        bool allFiveSucceeded = true;
        for (int burst = 0; burst < 5; ++burst) {
            if (combat.tryActivateWallBreak(map, player, effects, shake) < 0) {
                allFiveSucceeded = false;
            }
        }
        expectTrue(allFiveSucceeded && player.segments == 3 && Config::WALL_BREAK_COOLDOWN == 0.0f,
                   "Phim F: Hoan toan KHONG CO thoi gian hoi chieu (no lien tuc 5 lan tuc thi tu 8 dot ve 3 dot)");
    }

    // -------------------------------------------------------------------------
    // KIỂM THỬ 3: Hiệu ứng gạch vỡ mờ dần và biến mất đúng mốc 0.5 giây
    // -------------------------------------------------------------------------
    {
        EffectManager effects;
        effects.spawnWallCrumbleEffect({100.0f, 100.0f}, Config::TILE_SIZE);
        expectTrue(effects.getActiveWorldEffectCount() > 0,
                   "VFX Gach vo: Sinh manh gach tai cho khi tuong vo");
        effects.update(0.45f);
        expectTrue(effects.getActiveWorldEffectCount() > 0,
                   "VFX Gach vo: Tai 0.45s manh gach van dang mo dan");
        effects.update(0.07f); // Tổng 0.52s > 0.50s
        expectTrue(effects.getActiveWorldEffectCount() == 0,
                   "VFX Gach vo: Bien mat hoan toan sau 0.5 giay");
    }

    // -------------------------------------------------------------------------
    // KIỂM THỬ 4: Màn 2 Ngắm bắn Boss, thoát ngay sau 1 viên + hồi chiêu 5s + kẹp rìa 5 ô
    // -------------------------------------------------------------------------
    {
        MockMap map;
        MockPlayer player;
        MockBoss boss;
        EffectManager effects;
        ScreenShake shake;
        CombatSystem combat;
        combat.resetAll(GameStage::Stage2_BossFight);

        player.head = {5, 5}; // Tâm tại (220, 220)
        // Đặt Boss trong tầm 3.5 ô: góc trên trái tại ô (8, 4) -> X=[320..400], Y=[160..240]
        expectTrue(combat.tryToggleAim(player, &boss, effects),
                   "Man 2: Nhan Space bat che do ngam thanh cong");
        expectTrue(approxEqual(combat.getTimeScale(), Config::SLOW_MOTION_FACTOR) &&
                       approxEqual(combat.getTimeScale(), 0.10f) &&
                       approxEqual(combat.getAimTimeRemaining(), 10.0f),
                   "Man 2: Bat Slow Motion timeScale = 0.10f va thoi gian Toggle mac dinh = 10.0 giay");

        // Bắn trúng Boss tại (350, 210)
        const auto hitShot = combat.tryFireShot({350.0f, 210.0f}, map, player, &boss, effects, shake);
        expectTrue(hitShot.fired && hitShot.hitBoss && boss.hp == 85 && player.ammo == 9,
                   "Man 2: Ban trung Boss gay dung 15 HP sat thuong va tru 1 vien dan");
        expectTrue(!combat.isAiming() &&
                       combat.getAimState() == CombatSystem::AimState::Cooldown &&
                       approxEqual(combat.getCooldownRemaining(), 5.0f) &&
                       approxEqual(combat.getTimeScale(), 1.0f),
                   "Man 2: Ban xong 1 vien la tu thoat ngam ngay va vao hoi chieu dung 5 giay");

        // Trong lúc hồi chiêu không thể bật lại Space
        expectTrue(!combat.tryToggleAim(player, &boss, effects),
                   "Man 2: Khoa Space trong thoi gian hoi chieu 5 giay");
        combat.update(5.05f, player, &boss, effects);
        expectTrue(combat.getAimState() == CombatSystem::AimState::Ready,
                   "Man 2: Het 5 giay hoi chieu he thong san sang tro lai");

        // Dời Boss ra xa 7 ô (ngoài tầm 5 ô = 200px) và click thẳng lên Boss
        boss.box = sf::FloatRect({13.0f * Config::TILE_SIZE, 5.0f * Config::TILE_SIZE},
                                 {2.0f * Config::TILE_SIZE, 2.0f * Config::TILE_SIZE});
        combat.tryToggleAim(player, &boss, effects);
        const auto farShot = combat.tryFireShot({14.0f * Config::TILE_SIZE, 5.5f * Config::TILE_SIZE},
                                                map, player, &boss, effects, shake);
        const sf::Vector2f delta = farShot.impactWorld - farShot.originWorld;
        const float travelDist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        expectTrue(farShot.fired && farShot.clampedToMaxRange && !farShot.hitBoss &&
                       player.ammo == 8 && boss.hp == 85 &&
                       approxEqual(travelDist, Config::AIM_RANGE_PIXELS, 0.05f),
                   "Man 2: Click ngoai tam 5 o van ton 1 dan, vien dan dung dung tai ria 5 o va truot Boss o xa");
    }

    // -------------------------------------------------------------------------
    // KIỂM THỬ 5: Công thức Rung màn hình giới hạn mức 6/10 & tổng cấp số nhân (u1=1.5s, q=0.30)
    // -------------------------------------------------------------------------
    {
        ScreenShake shake;
        shake.setEnabled(true);

        shake.trigger(); // Lần 1: u1 = 1.50s
        expectTrue(approxEqual(shake.getTotalAccumulatedDuration(), 1.50f),
                   "ScreenShake: Lan 1 co thoi gian u1 = 1.50 giay");

        shake.trigger(); // Lần 2: + 1.50 * 0.30 = +0.45s => Tổng 1.95s
        expectTrue(approxEqual(shake.getTotalAccumulatedDuration(), 1.95f),
                   "ScreenShake: Lan 2 tich luy cap so nhan = 1.50 + 0.45 = 1.95 giay");

        shake.trigger(); // Lần 3: + 1.50 * 0.09 = +0.135s => Tổng 2.085s
        expectTrue(approxEqual(shake.getTotalAccumulatedDuration(), 2.085f),
                   "ScreenShake: Lan 3 tich luy cap so nhan = 2.085 giay");

        // Kích hoạt dồn liên tục thêm 20 lần nữa để kiểm tra trần cường độ 6/10
        for (int i = 0; i < 20; ++i) {
            shake.trigger();
        }
        expectTrue(approxEqual(shake.getCurrentLevel(), VfxStyle::SHAKE_MAX_LEVEL_CAP) &&
                       shake.getCurrentLevel() <= 6.0f,
                   "ScreenShake: Cong don nhieu lan van khoa cung o muc toi da 6/10");
        expectTrue(shake.getTotalAccumulatedDuration() < (1.50f / (1.0f - 0.30f)) + 0.001f,
                   "ScreenShake: Tong thoi gian cap so nhan hoi tu duoi 2.143 giay");
    }

    // -------------------------------------------------------------------------
    // KIỂM THỬ 6: Ràng buộc sinh Táo (cấm sát mép/góc, sinh gần đầu rắn) & Nhịp mọc tường nhanh
    // -------------------------------------------------------------------------
    {
        MockMap map;
        // Tạo tường bao quanh bản đồ giống thực tế
        for (int c = 0; c < Config::MAP_COLS; ++c) {
            map.set(c, 0, MockMap::Fixed);
            map.set(c, Config::MAP_ROWS - 1, MockMap::Fixed);
        }
        for (int r = 0; r < Config::MAP_ROWS; ++r) {
            map.set(0, r, MockMap::Fixed);
            map.set(Config::MAP_COLS - 1, r, MockMap::Fixed);
        }

        std::mt19937 rng(999u);
        const sf::Vector2i headPos{10, 9};
        bool neverOnEdgeOrCorner = true;
        bool alwaysNearbyHead = true;

        for (int trial = 0; trial < 300; ++trial) {
            const sf::Vector2i spawned = SpawnRules::pickNearbySafeItemCell(
                headPos,
                {-1, -1},
                rng,
                [&map](int c, int r) { return map.get(c, r) != MockMap::Empty; },
                [headPos](int c, int r) { return c == headPos.x && r == headPos.y; }
            );
            if (!SpawnRules::isInsideSafeInnerMargin(spawned.x, spawned.y)) {
                neverOnEdgeOrCorner = false;
            }
            const int dist = std::max(std::abs(spawned.x - headPos.x), std::abs(spawned.y - headPos.y));
            if (dist < Config::APPLE_MIN_DIST_FROM_HEAD || dist > Config::APPLE_MAX_DIST_FROM_HEAD) {
                alwaysNearbyHead = false;
            }
        }

        expectTrue(neverOnEdgeOrCorner,
                   "SpawnRules: Qua 300 lan random, tao TUYET DOI KHONG xuat hien o mep va 4 goc ban do");
        expectTrue(alwaysNearbyHead,
                   "SpawnRules: Tao luon xuat hien ngau nhien gan dau ran (trong pham vi 2..6 o)");
        expectTrue(SpawnRules::computeWallSpawnInterval(0) <= 1.20f &&
                       SpawnRules::computeWallSpawnInterval(8) < SpawnRules::computeWallSpawnInterval(0),
                   "SpawnRules: Tuong moc nhanh hon (1.15s) va tang toc dan khi ran an them tao");
    }

    if (g_failedAssertions == 0) {
        std::cout << "\n==> TAT CA BAI KIEM THU LOGIC DEU DAT 100%!\n";
        return EXIT_SUCCESS;
    }
    std::cerr << "\n==> CO " << g_failedAssertions << " KIEM THU THAT BAI!\n";
    return EXIT_FAILURE;
}
