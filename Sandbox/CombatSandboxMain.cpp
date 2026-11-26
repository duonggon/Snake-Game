#include "Combat/CombatInterfaces.h"
#include "Combat/CombatSystem.h"
#include "Core/Config.h"
#include "Core/SpawnRules.h"
#include "Effects/EffectManager.h"
#include "Effects/ScreenShake.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

#include <algorithm>
#include <cmath>
#include <deque>
#include <filesystem>
#include <iomanip>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// =============================================================================
// CHƯƠNG TRÌNH CHƠI THỬ ĐỘC LẬP (Sandbox/CombatSandboxMain.cpp)
// Phụ trách phần chiến đấu & VFX: Huỳnh Nhựt Cường
//
// Giả lập đầy đủ môi trường bản đồ 24x18, Rắn, Táo, Tường ngẫu nhiên và Boss 2x2
// để kiểm tra trực quan toàn bộ tính năng trước khi ghép vào nhánh chung:
// - Màn 1 (Sinh tồn) : Rắn ăn táo dài đốt, tường mọc ngẫu nhiên, bấm F phá tường
//                      (trừ 1 đốt, cần > 3 đốt). Đâm vào tường là Rắn DIE ngay!
//                      Khóa súng hoàn toàn ở Màn 1.
// - Màn 2 (Đánh Boss): Tự chuyển khi ăn đủ 5 táo (hoặc nhấn phím Tab bất kỳ lúc nào).
//                      Mở súng: nhấn Space (Toggle Slow-Mo 30%) + Click trái bắn 1 viên
//                      (đường đạn nét đứt tới tối đa rìa 5 ô, hồi chiêu 5s).
// =============================================================================
namespace {

enum class CellType {
    Empty = 0,
    FixedWall = 1,
    DestructibleWall = 2
};

// -----------------------------------------------------------------------------
// Bản đồ giả lập 24 x 18 ô thực thi đúng giao diện ICombatMap
// -----------------------------------------------------------------------------
class SandboxMap final : public ICombatMap {
public:
    SandboxMap() : m_cells(Config::MAP_COLS * Config::MAP_ROWS, CellType::Empty), m_rng(42u) {
        resetMap();
    }

    void resetMap() {
        std::fill(m_cells.begin(), m_cells.end(), CellType::Empty);

        // Tạo tường bao cố định xung quanh mép bản đồ 24 x 18
        for (int c = 0; c < Config::MAP_COLS; ++c) {
            setCell(c, 0, CellType::FixedWall);
            setCell(c, Config::MAP_ROWS - 1, CellType::FixedWall);
        }
        for (int r = 0; r < Config::MAP_ROWS; ++r) {
            setCell(0, r, CellType::FixedWall);
            setCell(Config::MAP_COLS - 1, r, CellType::FixedWall);
        }

        // Đặt 4 trụ tường cố định nhỏ ở bốn góc trong để kiểm tra không phá nhầm FixedWall
        setCell(5, 4, CellType::FixedWall);
        setCell(18, 4, CellType::FixedWall);
        setCell(5, 13, CellType::FixedWall);
        setCell(18, 13, CellType::FixedWall);

        // Rải một số cụm tường gạch có thể phá hủy ban đầu (chừa hành lang xuất phát của rắn)
        for (int i = 0; i < 20; ++i) {
            spawnRandomDestructibleWall({10, 9}, {-1, -1});
        }
    }

    bool isInsideBounds(int col, int row) const override {
        return col >= 0 && col < Config::MAP_COLS && row >= 0 && row < Config::MAP_ROWS;
    }

    bool isDestructibleWall(int col, int row) const override {
        return isInsideBounds(col, row) && getCell(col, row) == CellType::DestructibleWall;
    }

    bool isAnyWall(int col, int row) const {
        if (!isInsideBounds(col, row)) {
            return true;
        }
        const CellType t = getCell(col, row);
        return t == CellType::FixedWall || t == CellType::DestructibleWall;
    }

    void destroyWall(int col, int row) override {
        if (isDestructibleWall(col, row)) {
            setCell(col, row, CellType::Empty);
        }
    }

    sf::Vector2f gridToWorldCenter(int col, int row) const override {
        return {
            (static_cast<float>(col) + 0.5f) * Config::TILE_SIZE,
            Config::HUD_TOP_HEIGHT + (static_cast<float>(row) + 0.5f) * Config::TILE_SIZE
        };
    }

    template <typename IsSnakeOccupiedFn>
    void spawnRandomDestructibleWall(
        sf::Vector2i snakeHead,
        sf::Vector2i applePos,
        IsSnakeOccupiedFn&& isSnakeAt
    ) {
        std::uniform_int_distribution<int> colDist(1, Config::MAP_COLS - 2);
        std::uniform_int_distribution<int> rowDist(1, Config::MAP_ROWS - 2);
        std::uniform_int_distribution<int> clusterChance(0, 99);

        const auto canPlaceBrickAt = [&](int x, int y) {
            if (x < 1 || x >= Config::MAP_COLS - 1 || y < 1 || y >= Config::MAP_ROWS - 1) {
                return false;
            }
            if (getCell(x, y) != CellType::Empty || isSnakeAt(x, y)) {
                return false;
            }
            if (std::abs(x - snakeHead.x) <= 2 && std::abs(y - snakeHead.y) <= 1) {
                return false;
            }
            if (applePos.x >= 0 && std::abs(x - applePos.x) <= 1 && std::abs(y - applePos.y) <= 1) {
                return false;
            }
            return true;
        };

        for (int attempt = 0; attempt < 50; ++attempt) {
            const int c = colDist(m_rng);
            const int r = rowDist(m_rng);
            if (!canPlaceBrickAt(c, r)) {
                continue;
            }
            setCell(c, r, CellType::DestructibleWall);

            // Tạo cụm tường nhỏ 2..3 viên liền kề (60% xác suất) để người chơi phá nhiều tường cùng lúc trong 3x3!
            if (clusterChance(m_rng) < 60) {
                if (canPlaceBrickAt(c + 1, r)) {
                    setCell(c + 1, r, CellType::DestructibleWall);
                }
                if (clusterChance(m_rng) < 35 && canPlaceBrickAt(c, r + 1)) {
                    setCell(c, r + 1, CellType::DestructibleWall);
                }
            }
            return;
        }
    }

    void spawnRandomDestructibleWall(sf::Vector2i snakeHead, sf::Vector2i applePos) {
        spawnRandomDestructibleWall(snakeHead, applePos, [snakeHead](int c, int r) {
            return std::abs(c - snakeHead.x) <= 4 && std::abs(r - snakeHead.y) <= 1;
        });
    }

    CellType getCell(int col, int row) const {
        return m_cells[static_cast<std::size_t>(row * Config::MAP_COLS + col)];
    }

    void setCell(int col, int row, CellType type) {
        m_cells[static_cast<std::size_t>(row * Config::MAP_COLS + col)] = type;
    }

    void render(sf::RenderTarget& target) const {
        for (int r = 0; r < Config::MAP_ROWS; ++r) {
            for (int c = 0; c < Config::MAP_COLS; ++c) {
                const sf::Vector2f topLeft = {
                    static_cast<float>(c) * Config::TILE_SIZE,
                    Config::HUD_TOP_HEIGHT + static_cast<float>(r) * Config::TILE_SIZE
                };

                // Nền bàn cờ tối dịu để làm nổi bật hiệu ứng chiến đấu
                sf::RectangleShape floorTile({Config::TILE_SIZE, Config::TILE_SIZE});
                floorTile.setPosition(topLeft);
                floorTile.setFillColor(((r + c) % 2 == 0)
                                           ? sf::Color(24, 30, 38)
                                           : sf::Color(28, 35, 44));
                target.draw(floorTile);

                const CellType cell = getCell(c, r);
                if (cell == CellType::FixedWall) {
                    sf::RectangleShape wallRect({Config::TILE_SIZE - 2.0f, Config::TILE_SIZE - 2.0f});
                    wallRect.setPosition({topLeft.x + 1.0f, topLeft.y + 1.0f});
                    wallRect.setFillColor(sf::Color(58, 66, 80));
                    wallRect.setOutlineThickness(1.0f);
                    wallRect.setOutlineColor(sf::Color(92, 104, 122));
                    target.draw(wallRect);

                    // Ký hiệu dấu X kim loại trên các trụ tường cố định bên trong bản đồ (không thể phá)
                    const bool isInnerPillar =
                        (c > 0 && c < Config::MAP_COLS - 1 && r > 0 && r < Config::MAP_ROWS - 1);
                    if (isInnerPillar) {
                        const sf::Vector2f center = gridToWorldCenter(c, r);
                        sf::RectangleShape diag({Config::TILE_SIZE * 0.78f, 2.5f});
                        diag.setOrigin({Config::TILE_SIZE * 0.39f, 1.25f});
                        diag.setPosition(center);
                        diag.setFillColor(sf::Color(135, 150, 175, 200));
                        diag.setRotation(sf::degrees(45.0f));
                        target.draw(diag);
                        diag.setRotation(sf::degrees(-45.0f));
                        target.draw(diag);
                    }
                } else if (cell == CellType::DestructibleWall) {
                    sf::RectangleShape brickRect({Config::TILE_SIZE - 3.0f, Config::TILE_SIZE - 3.0f});
                    brickRect.setPosition({topLeft.x + 1.5f, topLeft.y + 1.5f});
                    brickRect.setFillColor(sf::Color(186, 98, 52));
                    brickRect.setOutlineThickness(1.5f);
                    brickRect.setOutlineColor(sf::Color(225, 138, 82));
                    target.draw(brickRect);

                    // Đường mạch gạch ngang nhỏ để dễ phân biệt tường phá được với tường cố định
                    sf::RectangleShape mortarLine({Config::TILE_SIZE - 8.0f, 2.0f});
                    mortarLine.setPosition({topLeft.x + 4.0f, topLeft.y + Config::TILE_SIZE * 0.5f});
                    mortarLine.setFillColor(sf::Color(120, 58, 28));
                    target.draw(mortarLine);
                }
            }
        }
    }

private:
    std::vector<CellType> m_cells;
    std::mt19937 m_rng;
};

// -----------------------------------------------------------------------------
// Con rắn giả lập thực thi ICombatPlayer (Ăn táo tăng đốt, va tường là DIE ngay)
// -----------------------------------------------------------------------------
class SandboxSnake final : public ICombatPlayer {
public:
    SandboxSnake() {
        reset();
    }

    void reset() {
        m_alive = true;
        m_ammo = 6;
        m_moveAccumulator = 0.0f;
        m_direction = {1, 0};
        m_nextDirection = {1, 0};
        m_segments.clear();
        // Khởi đầu 5 đốt để người chơi thử được ngay kỹ năng phá tường phím F (> 3 đốt)
        for (int i = 0; i < 5; ++i) {
            m_segments.push_back({10 - i, 9});
        }
    }

    void requestDirection(sf::Vector2i newDir) {
        // Chặn quay đầu ngược 180 độ trực tiếp
        if (newDir.x != -m_direction.x || newDir.y != -m_direction.y) {
            m_nextDirection = newDir;
        }
    }

    // Trả về true nếu rắn vừa đâm phải tường hoặc tự cắn thân dẫn tới DIE
    bool updateMovement(
        float scaledDeltaTime,
        const SandboxMap& map,
        sf::Vector2i& appleGridPos,
        sf::Vector2i& ammoBoxGridPos,
        bool isBossStage,
        int& applesEatenCounter
    ) {
        if (!m_alive) {
            return false;
        }

        constexpr float STEP_INTERVAL = 0.165f; // Tốc độ di chuyển 1 ô của rắn
        m_moveAccumulator += scaledDeltaTime;
        if (m_moveAccumulator < STEP_INTERVAL) {
            return false;
        }
        m_moveAccumulator -= STEP_INTERVAL;

        m_direction = m_nextDirection;
        const sf::Vector2i nextHead = m_segments.front() + m_direction;

        // QUY TẮC QUAN TRỌNG: Rắn va phải bất kỳ bức tường nào là DIE luôn!
        if (map.isAnyWall(nextHead.x, nextHead.y)) {
            m_alive = false;
            return true;
        }

        // Kiểm tra tự va chạm thân rắn
        for (std::size_t i = 0; i + 1 < m_segments.size(); ++i) {
            if (m_segments[i] == nextHead) {
                m_alive = false;
                return true;
            }
        }

        m_segments.push_front(nextHead);

        // Kiểm tra ăn táo -> dài thêm 1 đốt (tối đa 12 đốt)
        if (nextHead == appleGridPos) {
            applesEatenCounter += 1;
            appleGridPos = {-1, -1};
            if (static_cast<int>(m_segments.size()) > Config::MAX_SNAKE_LENGTH) {
                m_segments.pop_back();
            }
        } else {
            m_segments.pop_back();
        }

        // Ở Màn 2 nếu ăn hộp đạn -> cộng thêm 3 viên đạn
        if (isBossStage && nextHead == ammoBoxGridPos) {
            m_ammo += 3;
            ammoBoxGridPos = {-1, -1};
        }

        return false;
    }

    void addDebugResources(int extraSegments, int extraAmmo) {
        for (int i = 0; i < extraSegments && static_cast<int>(m_segments.size()) < Config::MAX_SNAKE_LENGTH; ++i) {
            m_segments.push_back(m_segments.back());
        }
        m_ammo += extraAmmo;
    }

    bool occupiesCell(int col, int row) const {
        for (const auto& seg : m_segments) {
            if (seg.x == col && seg.y == row) {
                return true;
            }
        }
        return false;
    }

    bool isAlive() const override {
        return m_alive;
    }

    sf::Vector2i getHeadGridPos() const override {
        return m_segments.empty() ? sf::Vector2i{1, 1} : m_segments.front();
    }

    int getSegmentCount() const override {
        return static_cast<int>(m_segments.size());
    }

    void shrinkSegments(int count) override {
        for (int i = 0; i < count && m_segments.size() > 1; ++i) {
            m_segments.pop_back();
        }
    }

    int getAmmo() const override {
        return m_ammo;
    }

    bool consumeAmmo(int count) override {
        if (m_ammo < count) {
            return false;
        }
        m_ammo -= count;
        return true;
    }

    void render(sf::RenderTarget& target, const SandboxMap& map) const {
        for (std::size_t i = 0; i < m_segments.size(); ++i) {
            const sf::Vector2f center = map.gridToWorldCenter(m_segments[i].x, m_segments[i].y);
            const bool isHead = (i == 0);
            const float pad = isHead ? 3.0f : 5.5f;
            const sf::Vector2f boxSize = {Config::TILE_SIZE - pad * 2.0f, Config::TILE_SIZE - pad * 2.0f};

            sf::RectangleShape segRect(boxSize);
            segRect.setOrigin(boxSize * 0.5f);
            segRect.setPosition(center);

            if (!m_alive) {
                segRect.setFillColor(isHead ? sf::Color(180, 65, 65) : sf::Color(115, 55, 55));
            } else if (isHead) {
                segRect.setFillColor(sf::Color(90, 235, 125));
                segRect.setOutlineThickness(2.0f);
                segRect.setOutlineColor(sf::Color(210, 255, 220));
            } else {
                // Đổi sắc độ nhẹ ở các đốt trên 3 để người chơi nhìn thoáng là biết còn đốt để nổ F
                const bool spendableSegment = (i >= static_cast<std::size_t>(Config::MIN_SNAKE_LENGTH_FOR_WALL_BREAK));
                segRect.setFillColor(spendableSegment
                                         ? sf::Color(65, 200, 145)
                                         : sf::Color(48, 145, 88));
            }
            target.draw(segRect);
        }
    }

private:
    bool m_alive = true;
    int m_ammo = 6;
    float m_moveAccumulator = 0.0f;
    sf::Vector2i m_direction{1, 0};
    sf::Vector2i m_nextDirection{1, 0};
    std::deque<sf::Vector2i> m_segments;
};

// -----------------------------------------------------------------------------
// Quả cầu Boss 2 x 2 ô giả lập thực thi ICombatBoss (100 HP, tự lăn & nháy khi trúng đạn)
// -----------------------------------------------------------------------------
class SandboxBoss final : public ICombatBoss {
public:
    SandboxBoss() {
        reset(false);
    }

    void reset(bool activeNow) {
        m_active = activeNow;
        m_hp = Config::BOSS_MAX_HP;
        m_worldTopLeft = {
            15.0f * Config::TILE_SIZE,
            Config::HUD_TOP_HEIGHT + 4.0f * Config::TILE_SIZE
        };
        m_velocity = {135.0f, 0.0f};
        m_axisSwitchTimer = 2.2f;
        m_hitFlashTimer = 0.0f;
        m_spinAngleDeg = 0.0f;
    }

    void update(float scaledDeltaTime, const SandboxMap& map) {
        if (!m_active || !isAlive()) {
            return;
        }

        if (m_hitFlashTimer > 0.0f) {
            m_hitFlashTimer = std::max(0.0f, m_hitFlashTimer - scaledDeltaTime);
        }

        // Boss tăng tốc khi máu còn <= 50 HP (khớp mô tả của Lê Đào Tấn Đạt)
        const float rageMultiplier = (m_hp <= 50) ? 1.45f : 1.0f;
        const sf::Vector2f step = m_velocity * rageMultiplier * scaledDeltaTime;
        m_worldTopLeft += step;
        m_spinAngleDeg += ((m_velocity.x + m_velocity.y) >= 0.0f ? 1.0f : -1.0f) * 190.0f * rageMultiplier * scaledDeltaTime;

        // Giữ quả cầu Boss 2x2 bên trong tường bao bản đồ
        const float minX = 1.0f * Config::TILE_SIZE;
        const float maxX = static_cast<float>(Config::MAP_COLS - 1 - Config::BOSS_SIZE_TILES) * Config::TILE_SIZE;
        const float minY = Config::HUD_TOP_HEIGHT + 1.0f * Config::TILE_SIZE;
        const float maxY = Config::HUD_TOP_HEIGHT +
                           static_cast<float>(Config::MAP_ROWS - 1 - Config::BOSS_SIZE_TILES) * Config::TILE_SIZE;

        if (m_worldTopLeft.x < minX) {
            m_worldTopLeft.x = minX;
            m_velocity.x = std::abs(m_velocity.x);
        } else if (m_worldTopLeft.x > maxX) {
            m_worldTopLeft.x = maxX;
            m_velocity.x = -std::abs(m_velocity.x);
        }

        if (m_worldTopLeft.y < minY) {
            m_worldTopLeft.y = minY;
            m_velocity.y = std::abs(m_velocity.y);
        } else if (m_worldTopLeft.y > maxY) {
            m_worldTopLeft.y = maxY;
            m_velocity.y = -std::abs(m_velocity.y);
        }

        // Đổi phương lăn X <-> Y định kỳ
        m_axisSwitchTimer -= scaledDeltaTime;
        if (m_axisSwitchTimer <= 0.0f) {
            m_axisSwitchTimer = 2.0f;
            if (std::abs(m_velocity.x) > 0.1f) {
                const float sign = (m_worldTopLeft.y < (minY + maxY) * 0.5f) ? 1.0f : -1.0f;
                m_velocity = {0.0f, 135.0f * sign};
            } else {
                const float sign = (m_worldTopLeft.x < (minX + maxX) * 0.5f) ? 1.0f : -1.0f;
                m_velocity = {135.0f * sign, 0.0f};
            }
        }

        // Boss lăn ngang qua tường gạch có thể ủi vỡ tường (tránh kẹt Boss trong Sandbox)
        const sf::FloatRect box = getWorldHitbox();
        const int cStart = static_cast<int>(box.position.x / Config::TILE_SIZE);
        const int cEnd = static_cast<int>((box.position.x + box.size.x - 1.0f) / Config::TILE_SIZE);
        const int rStart = static_cast<int>((box.position.y - Config::HUD_TOP_HEIGHT) / Config::TILE_SIZE);
        const int rEnd = static_cast<int>((box.position.y + box.size.y - 1.0f - Config::HUD_TOP_HEIGHT) / Config::TILE_SIZE);
        (void)map;
        (void)cStart;
        (void)cEnd;
        (void)rStart;
        (void)rEnd;
    }

    bool isActive() const override {
        return m_active;
    }

    bool isAlive() const override {
        return m_hp > 0;
    }

    int getHp() const {
        return m_hp;
    }

    sf::FloatRect getWorldHitbox() const override {
        const float sizePx = static_cast<float>(Config::BOSS_SIZE_TILES) * Config::TILE_SIZE;
        return sf::FloatRect(m_worldTopLeft, {sizePx, sizePx});
    }

    bool takeDamage(int damageAmount) override {
        if (!m_active || m_hp <= 0) {
            return false;
        }
        m_hp = std::max(0, m_hp - damageAmount);
        m_hitFlashTimer = 0.16f;
        return m_hp == 0;
    }

    void render(sf::RenderTarget& target) const {
        if (!m_active || !isAlive()) {
            return;
        }

        const sf::FloatRect box = getWorldHitbox();
        const float radius = box.size.x * 0.47f;
        const sf::Vector2f center = {box.position.x + box.size.x * 0.5f, box.position.y + box.size.y * 0.5f};

        sf::CircleShape orb(radius, 36);
        orb.setOrigin({radius, radius});
        orb.setPosition(center);

        const bool flashing = (m_hitFlashTimer > 0.0f);
        if (flashing) {
            orb.setFillColor(sf::Color(255, 245, 235));
            orb.setOutlineColor(sf::Color(255, 95, 75));
        } else if (m_hp <= 50) {
            orb.setFillColor(sf::Color(220, 62, 72));
            orb.setOutlineColor(sf::Color(255, 165, 90));
        } else {
            orb.setFillColor(sf::Color(175, 65, 205));
            orb.setOutlineColor(sf::Color(235, 145, 255));
        }
        orb.setOutlineThickness(3.5f);
        target.draw(orb);

        // Chữ thập lõi xoay bên trong quả cầu Boss để thấy rõ hiệu ứng Slow Motion 30%
        sf::RectangleShape coreCross({radius * 1.35f, 6.0f});
        coreCross.setOrigin({radius * 0.675f, 3.0f});
        coreCross.setPosition(center);
        coreCross.setRotation(sf::degrees(m_spinAngleDeg));
        coreCross.setFillColor(sf::Color(255, 230, 140, 210));
        target.draw(coreCross);

        coreCross.setRotation(sf::degrees(m_spinAngleDeg + 90.0f));
        target.draw(coreCross);
    }

private:
    bool m_active = false;
    int m_hp = Config::BOSS_MAX_HP;
    sf::Vector2f m_worldTopLeft{};
    sf::Vector2f m_velocity{};
    float m_axisSwitchTimer = 2.0f;
    float m_hitFlashTimer = 0.0f;
    float m_spinAngleDeg = 0.0f;
};

sf::Vector2i findRandomFreeCell(
    const SandboxMap& map,
    const SandboxSnake& snake,
    sf::Vector2i avoidOtherItem,
    std::mt19937& rng
) {
    return SpawnRules::pickNearbySafeItemCell(
        snake.getHeadGridPos(),
        avoidOtherItem,
        rng,
        [&map](int c, int r) { return map.isAnyWall(c, r); },
        [&snake](int c, int r) { return snake.occupiesCell(c, r); }
    );
}

} // namespace

int main(int argc, char* argv[]) {
    // Nạp font chữ hệ thống để vẽ thanh thông tin HUD rõ ràng
    sf::Font uiFont;
    bool hasUiFont = false;
    for (const auto& path : {
             std::filesystem::path("Assets/Fonts/game_font.ttf"),
             std::filesystem::path("C:/Windows/Fonts/consolab.ttf"),
             std::filesystem::path("C:/Windows/Fonts/segoeui.ttf"),
             std::filesystem::path("C:/Windows/Fonts/arial.ttf")}) {
        std::error_code ec;
        if (std::filesystem::exists(path, ec) && uiFont.openFromFile(path)) {
            hasUiFont = true;
            break;
        }
    }

    SandboxMap map;
    SandboxSnake snake;
    SandboxBoss boss;

    EffectManager effects;
    effects.loadOptionalAssets();
    if (hasUiFont) {
        effects.setSharedFont(&uiFont);
    }

    ScreenShake screenShake;
    CombatSystem combat;

    std::mt19937 itemRng(2026u);
    sf::Vector2i ammoBoxPos{-1, -1};
    sf::Vector2i applePos = findRandomFreeCell(map, snake, ammoBoxPos, itemRng);
    int applesEaten = 0;
    float wallSpawnTimer = 0.0f;
    float statusNoticeTimer = 0.0f;
    std::string statusNotice = "MAN 1 (SINH TON): An tao tang dot, bam [Space] pha tuong 3x3 (-1 dot). Bam [Tab] sang Man 2 Boss!";

    const sf::View baseWorldView(
        sf::FloatRect({0.0f, 0.0f},
                      {static_cast<float>(Config::WINDOW_WIDTH), static_cast<float>(Config::WINDOW_HEIGHT)})
    );

    // Hàm vẽ chung toàn cảnh (dùng cho cả cửa sổ chơi trực tiếp & chụp ảnh minh họa)
    const auto renderSceneToTarget = [&](sf::RenderTarget& target, sf::Vector2f mouseWorld) {
        target.clear(sf::Color(16, 20, 26));

        // 1. Vẽ thế giới bản đồ với View đã áp dụng độ lệch rung nhẹ
        target.setView(screenShake.applyToView(baseWorldView));
        map.render(target);

        if (applePos.x >= 0) {
            const sf::Vector2f c = map.gridToWorldCenter(applePos.x, applePos.y);
            sf::CircleShape appleShape(Config::TILE_SIZE * 0.32f, 20);
            appleShape.setOrigin({Config::TILE_SIZE * 0.32f, Config::TILE_SIZE * 0.32f});
            appleShape.setPosition(c);
            appleShape.setFillColor(sf::Color(240, 68, 68));
            appleShape.setOutlineThickness(2.0f);
            appleShape.setOutlineColor(sf::Color(255, 185, 165));
            target.draw(appleShape);
        }

        if (combat.getGameStage() == GameStage::Stage2_BossFight && ammoBoxPos.x >= 0) {
            const sf::Vector2f c = map.gridToWorldCenter(ammoBoxPos.x, ammoBoxPos.y);
            sf::RectangleShape ammoShape({Config::TILE_SIZE * 0.52f, Config::TILE_SIZE * 0.52f});
            ammoShape.setOrigin({Config::TILE_SIZE * 0.26f, Config::TILE_SIZE * 0.26f});
            ammoShape.setPosition(c);
            ammoShape.setFillColor(sf::Color(245, 195, 55));
            ammoShape.setOutlineThickness(2.0f);
            ammoShape.setOutlineColor(sf::Color(255, 240, 165));
            target.draw(ammoShape);
        }

        snake.render(target, map);

        // Vẽ khung căn tầm 3x3 mờ quanh đầu rắn khi kỹ năng [F] sẵn sàng + đánh dấu sáng các tường nằm trong 3x3
        if (combat.canUseWallBreak(snake)) {
            const sf::Vector2i head = snake.getHeadGridPos();
            const sf::Vector2f headCenter = map.gridToWorldCenter(head.x, head.y);
            const float boxSidePx =
                static_cast<float>(Config::WALL_BREAK_HALF_EXTENT * 2 + 1) * Config::TILE_SIZE;

            sf::RectangleShape range3x3({boxSidePx, boxSidePx});
            range3x3.setOrigin({boxSidePx * 0.5f, boxSidePx * 0.5f});
            range3x3.setPosition(headCenter);
            range3x3.setFillColor(sf::Color(255, 200, 70, 14));
            range3x3.setOutlineThickness(1.5f);
            range3x3.setOutlineColor(sf::Color(255, 205, 85, 110));
            target.draw(range3x3);

            // Tô viền vàng rực cho MỌI bức tường gạch đang lọt trong phạm vi 3x3 (bấm F sẽ vỡ hết!)
            const int ext = Config::WALL_BREAK_HALF_EXTENT;
            for (int r = head.y - ext; r <= head.y + ext; ++r) {
                for (int c = head.x - ext; c <= head.x + ext; ++c) {
                    if (map.isDestructibleWall(c, r)) {
                        const sf::Vector2f wc = map.gridToWorldCenter(c, r);
                        sf::RectangleShape lockHighlight({Config::TILE_SIZE - 2.0f, Config::TILE_SIZE - 2.0f});
                        lockHighlight.setOrigin({(Config::TILE_SIZE - 2.0f) * 0.5f,
                                                 (Config::TILE_SIZE - 2.0f) * 0.5f});
                        lockHighlight.setPosition(wc);
                        lockHighlight.setFillColor(sf::Color(255, 235, 100, 45));
                        lockHighlight.setOutlineThickness(2.2f);
                        lockHighlight.setOutlineColor(sf::Color(255, 235, 95, 235));
                        target.draw(lockHighlight);
                    }
                }
            }
        }

        boss.render(target);

        combat.renderAimingWorldUI(target, mouseWorld, map, snake, &boss);
        effects.renderWorldEffects(target);

        // 2. Chuyển về View tĩnh để vẽ lớp phủ màu Slow Motion mượt & Thanh HUD
        target.setView(baseWorldView);
        effects.renderScreenOverlay(
            target,
            {static_cast<float>(Config::WINDOW_WIDTH), static_cast<float>(Config::WINDOW_HEIGHT)}
        );

        sf::RectangleShape hudBg({static_cast<float>(Config::WINDOW_WIDTH), Config::HUD_TOP_HEIGHT});
        hudBg.setPosition({0.0f, 0.0f});
        hudBg.setFillColor(sf::Color(14, 18, 26, 240));
        hudBg.setOutlineThickness(1.5f);
        hudBg.setOutlineColor(sf::Color(55, 72, 96));
        target.draw(hudBg);

        if (hasUiFont) {
            std::ostringstream line1;
            const bool stage2 = (combat.getGameStage() == GameStage::Stage2_BossFight);
            const char* wallSkillLabel = !snake.isAlive()
                                             ? " (RAN DA DIE)"
                                             : (combat.canUseWallBreak(snake)
                                                    ? " ([Space] Pha tuong 3x3)"
                                                    : " ([Space] Khoa <=3 dot)");
            line1 << (stage2 ? "[MAN 2: BOSS FIGHT]" : "[MAN 1: SINH TON]")
                  << " | Dot ran: " << snake.getSegmentCount()
                  << wallSkillLabel
                  << " | Tao: " << applesEaten;

            if (stage2) {
                line1 << " | Dan: " << snake.getAmmo()
                      << " | Boss HP: " << boss.getHp() << "/" << Config::BOSS_MAX_HP << " | ";
                if (combat.getAimState() == CombatSystem::AimState::Aiming) {
                    line1 << "SLOW-MO 10% (" << std::fixed << std::setprecision(1)
                          << combat.getAimTimeRemaining() << "s - Click Trai ban)";
                } else if (combat.getAimState() == CombatSystem::AimState::Cooldown) {
                    line1 << "Hoi chieu Ngam: " << std::fixed << std::setprecision(1)
                          << combat.getCooldownRemaining() << "s";
                } else {
                    line1 << "[Chuot Phai] Ngam";
                }
            } else {
                line1 << " | [Tab]: Sang Man 2 Boss | [G]: +Dot/+Dan | [R]: Choi lai";
            }

            sf::Text hudLine1(uiFont, line1.str(), 15);
            hudLine1.setFillColor(sf::Color(230, 240, 255));
            hudLine1.setPosition({12.0f, 10.0f});
            target.draw(hudLine1);

            const CombatStats& st = combat.getStats();
            std::ostringstream line2;
            if (statusNoticeTimer > 0.0f && !statusNotice.empty()) {
                line2 << ">> " << statusNotice;
            } else {
                line2 << "Thong ke -> Tuong pha: " << st.totalWallsDestroyed
                      << " | Dung Space: " << st.wallBreakUsedCount
                      << " | Ban: " << st.shotsHit << "/" << st.shotsFired
                      << " (" << std::fixed << std::setprecision(0) << st.getAccuracyPercent() << "%)"
                      << " | Rung: " << std::setprecision(1) << screenShake.getCurrentLevel() << "/10";
            }

            sf::Text hudLine2(uiFont, line2.str(), 14);
            hudLine2.setFillColor(!snake.isAlive()
                                      ? sf::Color(255, 105, 105)
                                      : sf::Color(165, 220, 185));
            hudLine2.setPosition({12.0f, 38.0f});
            target.draw(hudLine2);
        }
    };

    // Chế độ xuất nhanh 3 ảnh chụp minh họa thực tế phục vụ báo cáo / Walkthrough
    if (argc >= 3 && std::string(argv[1]) == "--capture-demo") {
        const std::filesystem::path outDir(argv[2]);
        std::filesystem::create_directories(outDir);
        sf::RenderTexture offscreen({Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT});

        // Ảnh 1: Màn 1 Sinh tồn - Nhấn F nổ vùng 3x3 quanh đầu rắn (10, 9), gạch nứt vỡ mờ dần 0.5s
        map.setCell(11, 8, CellType::DestructibleWall);
        map.setCell(11, 9, CellType::DestructibleWall);
        map.setCell(10, 8, CellType::DestructibleWall);
        const int broken = combat.tryActivateWallBreak(map, snake, effects, screenShake);
        statusNotice = "PHA TUONG 3x3 (-1 dot)! So o gach vo: " + std::to_string(broken) + " (mo dan 0.5s)";
        statusNoticeTimer = 2.0f;
        effects.update(0.14f);
        screenShake.update(0.14f);
        renderSceneToTarget(offscreen, {0.0f, 0.0f});
        offscreen.display();
        (void)offscreen.getTexture().copyToImage().saveToFile(outDir / "demo_stage1_wall_break.png");

        // Ảnh 2: Màn 2 Đánh Boss - Nhấn Space bật Slow Motion 30%, vòng ngắm 5 ô & tia nét đứt khóa Boss
        effects.clearAll();
        screenShake.reset();
        combat.setGameStage(GameStage::Stage2_BossFight, &effects);
        boss.reset(true);
        // Đưa Boss vào gần tầm 5 ô của đầu rắn (10, 9) để trình diễn khóa mục tiêu
        for (int step = 0; step < 8; ++step) {
            boss.update(-0.18f, map);
        }
        combat.tryToggleAim(snake, &boss, effects);
        for (int i = 0; i < 6; ++i) {
            effects.update(0.05f);
        }
        const sf::FloatRect bBox = boss.getWorldHitbox();
        const sf::Vector2f bossCenter = {bBox.position.x + bBox.size.x * 0.5f,
                                         bBox.position.y + bBox.size.y * 0.5f};
        statusNotice = "MAN 2 SLOW-MO 10% (10.0s): Vong ban kinh 5 o + Tia ngam net dut khoa trung Boss 2x2!";
        statusNoticeTimer = 2.0f;
        renderSceneToTarget(offscreen, bossCenter);
        offscreen.display();
        (void)offscreen.getTexture().copyToImage().saveToFile(outDir / "demo_stage2_slowmo_aim.png");

        // Ảnh 3: Khai hỏa đường đạn nét đứt nối từ đầu rắn -> viên đạn & vụ nổ tiêu diệt Boss
        for (int k = 0; k < 5; ++k) {
            (void)boss.takeDamage(15); // Giảm máu Boss xuống còn 25 HP trước phát bắn
        }
        (void)boss.takeDamage(10); // Còn đúng 15 HP để phát bắn kế tiếp tiêu diệt Boss!
        (void)combat.tryFireShot(bossCenter, map, snake, &boss, effects, screenShake);
        effects.update(0.22f);
        screenShake.update(0.22f);
        statusNotice = "DUONG DAN NET DUT (-15 HP) -> TIEU DIET & PHAT NO QUA CAU BOSS 2x2 (Hoi chieu 5.0s)!";
        statusNoticeTimer = 3.0f;
        renderSceneToTarget(offscreen, bossCenter);
        offscreen.display();
        (void)offscreen.getTexture().copyToImage().saveToFile(outDir / "demo_stage2_boss_explosion.png");
        return 0;
    }

    sf::RenderWindow window(
        sf::VideoMode({Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT}),
        "Snake OOP - Combat & VFX Sandbox (Huynh Nhut Cuong - SFML 3.1.0)"
    );
    window.setFramerateLimit(Config::TARGET_FPS);
    // Tắt lặp phím tự động của hệ điều hành để mỗi lần gõ F liên hoàn trừ đúng 1 đốt chuẩn xác
    window.setKeyRepeatEnabled(false);

    const auto switchToBossStage = [&]() {
        combat.setGameStage(GameStage::Stage2_BossFight, &effects);
        boss.reset(true);
        if (ammoBoxPos.x < 0) {
            ammoBoxPos = findRandomFreeCell(map, snake, applePos, itemRng);
        }
        statusNotice = "MAN 2 (BOSS FIGHT): An [Chuot Phai] bat Toggle Slow-Mo 10% (10s) + [Chuot Trai] ban Boss!";
        statusNoticeTimer = 4.5f;
    };

    const auto restartSandbox = [&]() {
        map.resetMap();
        snake.reset();
        boss.reset(false);
        effects.clearAll();
        screenShake.reset();
        combat.resetAll(GameStage::Stage1_Survival);
        applesEaten = 0;
        wallSpawnTimer = 0.0f;
        ammoBoxPos = {-1, -1};
        applePos = findRandomFreeCell(map, snake, ammoBoxPos, itemRng);
        statusNotice = "Da khoi dong lai Man 1! Bam [Space] pha tuong LIEN TUC (0s hoi chieu), [Tab] sang Man 2.";
        statusNoticeTimer = 3.5f;
    };

    const auto handleWallBreakAction = [&]() {
        if (!snake.isAlive()) {
            statusNotice = "Ran da DIE do va phai tuong! Nhan [R] de choi lai.";
            statusNoticeTimer = 2.2f;
            return;
        }
        const int destroyed = combat.tryActivateWallBreak(map, snake, effects, screenShake);
        if (destroyed < 0) {
            statusNotice = "KHONG THE PHA TUONG: Ran dang o muc toi thieu 3 dot (can >= 4 dot)!";
            statusNoticeTimer = 2.2f;
        } else {
            statusNotice = "PHA TUONG 3x3 (-1 dot, 0s hoi chieu)! So o gach vo: " + std::to_string(destroyed);
            statusNoticeTimer = 1.8f;
        }
    };

    sf::Clock frameClock;

    while (window.isOpen()) {
        // 1. Tính unscaledDeltaTime (giới hạn trần 0.05s để tránh nhảy bước khi kéo cửa sổ)
        const float unscaledDt = std::min(frameClock.restart().asSeconds(), 0.05f);
        bool fTriggeredByRawKey = false;

        // 2. Xử lý sự kiện bàn phím & chuột theo chuẩn SFML 3.1.0 (chống kẹt Unikey / Telex!)
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                continue;
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                const sf::Keyboard::Key code = keyPressed->code;
                const sf::Keyboard::Scancode scan = keyPressed->scancode;

                // Điều khiển hướng rắn WASD / Phím mũi tên (kiểm tra cả Scancode vật lý)
                if (code == sf::Keyboard::Key::W || scan == sf::Keyboard::Scancode::W ||
                    code == sf::Keyboard::Key::Up) {
                    snake.requestDirection({0, -1});
                } else if (code == sf::Keyboard::Key::S || scan == sf::Keyboard::Scancode::S ||
                           code == sf::Keyboard::Key::Down) {
                    snake.requestDirection({0, 1});
                } else if (code == sf::Keyboard::Key::A || scan == sf::Keyboard::Scancode::A ||
                           code == sf::Keyboard::Key::Left) {
                    snake.requestDirection({-1, 0});
                } else if (code == sf::Keyboard::Key::D || scan == sf::Keyboard::Scancode::D ||
                           code == sf::Keyboard::Key::Right) {
                    snake.requestDirection({1, 0});
                }
                // [PHÍM SPACE]: Kỹ năng phá tường 3x3 (0.0s hồi chiêu - bấm liên tục tức thì!)
                else if (code == Config::WALL_BREAK_KEY || scan == sf::Keyboard::Scancode::Space) {
                    fTriggeredByRawKey = true;
                    handleWallBreakAction();
                }
                // [PHÍM TAB]: Chuyển nhanh Màn 1 <-> Màn 2 để tiện kiểm thử
                else if (code == sf::Keyboard::Key::Tab) {
                    if (combat.getGameStage() == GameStage::Stage1_Survival) {
                        switchToBossStage();
                    } else {
                        combat.setGameStage(GameStage::Stage1_Survival, &effects);
                        boss.reset(false);
                        statusNotice = "Da chuyen ve MAN 1 (Sinh ton - dung Phim Space pha tuong).";
                        statusNoticeTimer = 2.5f;
                    }
                }
                // [PHÍM G]: Cộng nhanh +2 đốt rắn & +5 đạn để test liên tục
                else if (code == sf::Keyboard::Key::G) {
                    snake.addDebugResources(2, 5);
                    statusNotice = "Debug [+2 Dot ran, +5 Dan]!";
                    statusNoticeTimer = 1.5f;
                }
                // [PHÍM H]: Bật/Tắt nhanh hiệu ứng rung màn hình để so sánh cảm giác
                else if (code == sf::Keyboard::Key::H) {
                    screenShake.setEnabled(!screenShake.isEnabled());
                    statusNotice = std::string("Rung man hinh: ") + (screenShake.isEnabled() ? "BAT" : "TAT");
                    statusNoticeTimer = 1.8f;
                }
                // [PHÍM R]: Khởi động lại màn chơi
                else if (code == sf::Keyboard::Key::R || scan == sf::Keyboard::Scancode::R) {
                    restartSandbox();
                }
            }

            // [CHỐNG KẸT BỘ GÕ TIẾNG VIỆT UNIKEY / TELEX CHO WASD & R]:
            if (const auto* textEv = event->getIf<sf::Event::TextEntered>()) {
                const char32_t ch = textEv->unicode;
                (void)fTriggeredByRawKey;
                if (ch == U'ư' || ch == U'Ư' || ch == U'ă' || ch == U'Ă') {
                    snake.requestDirection({0, -1}); // Phím W bị Telex đổi thành ư / ă
                } else if (ch == U'â' || ch == U'Â') {
                    snake.requestDirection({-1, 0}); // Phím A lặp 2 lần thành â
                } else if (ch == U'đ' || ch == U'Đ') {
                    snake.requestDirection({1, 0});  // Phím D lặp 2 lần thành đ
                } else if (ch == U'á' || ch == U'Á' || ch == U'é' || ch == U'É' ||
                           ch == U'í' || ch == U'Í' || ch == U'ó' || ch == U'Ó' ||
                           ch == U'ú' || ch == U'Ú' || ch == U'ý' || ch == U'Ý' ||
                           ch == U'ắ' || ch == U'Ắ' || ch == U'ấ' || ch == U'Ấ' ||
                           ch == U'ế' || ch == U'Ế' || ch == U'ố' || ch == U'Ố' ||
                           ch == U'ớ' || ch == U'Ớ' || ch == U'ứ' || ch == U'Ứ') {
                    snake.requestDirection({0, 1});  // Phím S bị Telex đổi thành dấu sắc
                } else if (ch == U'ả' || ch == U'Ả' || ch == U'ẻ' || ch == U'Ẻ' ||
                           ch == U'ỉ' || ch == U'Ỉ' || ch == U'ỏ' || ch == U'Ỏ' ||
                           ch == U'ủ' || ch == U'Ủ' || ch == U'ỷ' || ch == U'Ỷ' ||
                           ch == U'ẳ' || ch == U'Ẳ' || ch == U'ẩ' || ch == U'Ẩ' ||
                           ch == U'ể' || ch == U'Ể' || ch == U'ổ' || ch == U'Ổ' ||
                           ch == U'ở' || ch == U'Ở' || ch == U'ử' || ch == U'Ử') {
                    restartSandbox();                // Phím R bị Telex đổi thành dấu hỏi
                }
            }

            // [XỬ LÝ NÚT CHUỘT Ở MÀN 2]:
            // - Click Chuột Phải: Toggle bật/tắt chế độ ngắm Slow Motion 10% (10.0s)
            // - Click Chuột Trái: Khai hỏa 1 phát đạn khi đang ngắm
            if (const auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Config::AIM_TOGGLE_MOUSE_BUTTON) {
                    if (combat.getGameStage() == GameStage::Stage1_Survival) {
                        statusNotice = "MAN 1 KHONG CO SUNG! An du 5 tao hoac bam [Tab] de sang Man 2 gap Boss.";
                        statusNoticeTimer = 2.2f;
                    } else if (combat.getAimState() == CombatSystem::AimState::Cooldown) {
                        std::ostringstream oss;
                        oss << "Dang hoi chieu ngam ban: con " << std::fixed << std::setprecision(1)
                            << combat.getCooldownRemaining() << "s!";
                        statusNotice = oss.str();
                        statusNoticeTimer = 1.5f;
                    } else if (snake.getAmmo() <= 0) {
                        statusNotice = "Het dan! Di an hop dan mau vang tren ban do (hoac bam [G] de cong dan test).";
                        statusNoticeTimer = 2.2f;
                    } else {
                        combat.tryToggleAim(snake, &boss, effects);
                    }
                } else if (mouseBtn->button == Config::FIRE_SHOT_MOUSE_BUTTON && combat.isAiming()) {
                    // Luôn quy đổi chuột qua baseWorldView tĩnh để không lệch tâm khi màn hình rung
                    const sf::Vector2f mouseWorld =
                        window.mapPixelToCoords(mouseBtn->position, baseWorldView);
                    const CombatSystem::ShotResult shot =
                        combat.tryFireShot(mouseWorld, map, snake, &boss, effects, screenShake);
                    if (shot.killedBoss) {
                        statusNotice = "VICTORY! BOSS 2x2 DA NO TUNG! Nhan [R] de choi lai.";
                        statusNoticeTimer = 6.0f;
                    }
                }
            }
        }

        // 3. Cập nhật CombatSystem, VFX & ScreenShake theo thời gian thực (unscaledDt)
        combat.clearFrameEvents();
        combat.update(unscaledDt, snake, &boss, effects);
        effects.update(unscaledDt);
        screenShake.update(unscaledDt);
        if (statusNoticeTimer > 0.0f) {
            statusNoticeTimer = std::max(0.0f, statusNoticeTimer - unscaledDt);
        }

        // 4. Tính scaledDt (bằng 30% khi đang bật Slow Motion ở Màn 2!) cho thế giới game
        const float scaledDt = unscaledDt * combat.getTimeScale();

        if (snake.isAlive()) {
            const bool isBossStage = (combat.getGameStage() == GameStage::Stage2_BossFight);
            const bool justDied = snake.updateMovement(
                scaledDt, map, applePos, ammoBoxPos, isBossStage, applesEaten
            );
            if (justDied) {
                const sf::Vector2i h = snake.getHeadGridPos();
                effects.spawnSnakeDeathBurst(map.gridToWorldCenter(h.x, h.y));
                screenShake.trigger();
                statusNotice = "RAN DA VA PHAI TUONG -> DIE LUON! Nhan [R] de choi lai.";
                statusNoticeTimer = 8.0f;
            }

            if (applePos.x < 0) {
                applePos = findRandomFreeCell(map, snake, ammoBoxPos, itemRng);
            }
            if (isBossStage && ammoBoxPos.x < 0) {
                ammoBoxPos = findRandomFreeCell(map, snake, applePos, itemRng);
            }

            // Tự động chuyển từ Màn 1 sang Màn 2 khi ăn đủ 5 quả táo
            if (!isBossStage && applesEaten >= 5) {
                switchToBossStage();
            }

            // Tường ngẫu nhiên mọc NHANH HƠN (1.15s cơ bản, giảm dần theo số táo ăn được!)
            wallSpawnTimer += scaledDt;
            const float currentWallInterval = SpawnRules::computeWallSpawnInterval(applesEaten);
            if (wallSpawnTimer >= currentWallInterval) {
                wallSpawnTimer = 0.0f;
                map.spawnRandomDestructibleWall(
                    snake.getHeadGridPos(),
                    applePos,
                    [&snake](int c, int r) { return snake.occupiesCell(c, r); }
                );
            }

            if (isBossStage) {
                boss.update(scaledDt, map);
            }
        }

        // =====================================================================
        // 5. VẼ KHUNG HÌNH (Áp dụng View rung cho bản đồ & View tĩnh cho HUD/Slow-Mo)
        // =====================================================================
        const sf::Vector2f mouseWorld =
            window.mapPixelToCoords(sf::Mouse::getPosition(window), baseWorldView);
        renderSceneToTarget(window, mouseWorld);
        window.display();
    }

    return 0;
}
