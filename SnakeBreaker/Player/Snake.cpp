#include "Snake.h"
#include <algorithm>
#include <cmath>

// ============================================================================
// File: Player/Snake.cpp
// Phụ trách: Thành viên 2 – Đào Duy Khôi (Snake & Apple System)
// Mô tả: Cài đặt chi tiết các phương thức của lớp Snake.
// ============================================================================

Snake::Snake()
    : Entity(sf::Vector2i(config::SAFE_SPAWN_X, config::SAFE_SPAWN_Y))
    , m_direction(Direction::RIGHT)
    , m_nextDirection(Direction::RIGHT)
    , m_speed(config::DEFAULT_SNAKE_SPEED)
    , m_moveTimer(0.0f)
    , m_pendingGrowth(0)
    , m_lives(config::INITIAL_LIVES)
    , m_energy(config::INITIAL_ENERGY)
    , m_ammo(0)
    , m_appleCount(0)
    , m_isAlive(true)
    , m_isInvincible(false)
    , m_invincibleTimer(0.0f)
    , m_blinkTimer(0.0f)
    , m_isVisible(true)
    , m_hasMovedThisFrame(false) {
    reset(sf::Vector2i(config::SAFE_SPAWN_X, config::SAFE_SPAWN_Y));
}

Snake::Snake(const sf::Vector2i& startGridPosition)
    : Entity(startGridPosition)
    , m_direction(Direction::RIGHT)
    , m_nextDirection(Direction::RIGHT)
    , m_speed(config::DEFAULT_SNAKE_SPEED)
    , m_moveTimer(0.0f)
    , m_pendingGrowth(0)
    , m_lives(config::INITIAL_LIVES)
    , m_energy(config::INITIAL_ENERGY)
    , m_ammo(0)
    , m_appleCount(0)
    , m_isAlive(true)
    , m_isInvincible(false)
    , m_invincibleTimer(0.0f)
    , m_blinkTimer(0.0f)
    , m_isVisible(true)
    , m_hasMovedThisFrame(false) {
    reset(startGridPosition);
}

void Snake::reset(const sf::Vector2i& startGridPosition) {
    m_direction = Direction::RIGHT;
    m_nextDirection = Direction::RIGHT;
    m_speed = config::DEFAULT_SNAKE_SPEED;
    m_moveTimer = 0.0f;
    m_pendingGrowth = 0;

    m_lives = config::INITIAL_LIVES;
    m_energy = config::INITIAL_ENERGY;
    m_ammo = 0;
    m_appleCount = 0;

    m_isAlive = true;
    m_isActive = true;
    m_isInvincible = false;
    m_invincibleTimer = 0.0f;
    m_blinkTimer = 0.0f;
    m_isVisible = true;
    m_hasMovedThisFrame = false;

    // Khởi tạo 3 đốt ban đầu xếp nằm ngang hướng sang phải
    m_body.clear();
    for (int i = 0; i < config::INITIAL_SNAKE_LENGTH; ++i) {
        int segX = std::max(1, startGridPosition.x - i);
        int segY = std::clamp(startGridPosition.y, 1, config::MAP_HEIGHT - 2);
        m_body.emplace_back(segX, segY);
    }

    setGridPosition(m_body.front());
}

void Snake::handleKeyPress(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::W:
        case sf::Keyboard::Up:
            setDirection(Direction::UP);
            break;
        case sf::Keyboard::S:
        case sf::Keyboard::Down:
            setDirection(Direction::DOWN);
            break;
        case sf::Keyboard::A:
        case sf::Keyboard::Left:
            setDirection(Direction::LEFT);
            break;
        case sf::Keyboard::D:
        case sf::Keyboard::Right:
            setDirection(Direction::RIGHT);
            break;
        default:
            break;
    }
}

void Snake::setDirection(Direction newDirection) {
    // Không cho phép quay đầu ngược 180 độ trực tiếp so với hướng di chuyển hiện tại
    if (!isOppositeDirection(m_direction, newDirection)) {
        m_nextDirection = newDirection;
    }
}

void Snake::update(float gameDeltaTime) {
    m_hasMovedThisFrame = false;

    if (!m_isAlive) {
        return;
    }

    // 1. Cập nhật thời gian bất tử và hiệu ứng nhấp nháy (1,5 giây sau khi mất mạng)
    if (m_isInvincible) {
        m_invincibleTimer -= gameDeltaTime;
        m_blinkTimer += gameDeltaTime;

        if (m_blinkTimer >= config::BLINK_INTERVAL) {
            m_blinkTimer -= config::BLINK_INTERVAL;
            m_isVisible = !m_isVisible; // Đảo trạng thái sáng/mờ để tạo hiệu ứng nhấp nháy
        }

        if (m_invincibleTimer <= 0.0f) {
            m_isInvincible = false;
            m_invincibleTimer = 0.0f;
            m_blinkTimer = 0.0f;
            m_isVisible = true; // Kết thúc bất tử thì hiển thị bình thường
        }
    }

    // 2. Cập nhật bộ đếm thời gian để rắn tự động di chuyển theo lưới
    if (m_speed <= 0.0f) {
        return;
    }

    const float stepInterval = 1.0f / m_speed;
    m_moveTimer += gameDeltaTime;

    if (m_moveTimer >= stepInterval) {
        m_moveTimer -= stepInterval;
        moveStep();
        m_hasMovedThisFrame = true;
    }
}

void Snake::moveStep() {
    if (!m_isAlive || m_body.empty()) {
        return;
    }

    // Áp dụng hướng đã chọn (đã được kiểm tra không ngược 180 độ)
    if (!isOppositeDirection(m_direction, m_nextDirection)) {
        m_direction = m_nextDirection;
    }

    const sf::Vector2i newHeadPosition = m_body.front() + getDirectionOffset(m_direction);

    // Chèn vị trí đầu mới vào đầu vector
    m_body.insert(m_body.begin(), newHeadPosition);

    // Kiểm tra nếu rắn đang chờ tăng chiều dài và chưa vượt quá 12 đốt
    if (m_pendingGrowth > 0 && static_cast<int>(m_body.size()) <= config::MAX_SNAKE_LENGTH) {
        --m_pendingGrowth;
    } else {
        // Không tăng chiều dài thì xóa đốt đuôi cuối cùng
        m_body.pop_back();
        if (static_cast<int>(m_body.size()) >= config::MAX_SNAKE_LENGTH) {
            m_pendingGrowth = 0; // Đảm bảo không tích lũy vượt quá 12 đốt
        }
    }

    // Đồng bộ tọa độ của lớp cơ sở Entity với đầu rắn
    setGridPosition(m_body.front());
}

void Snake::render(sf::RenderWindow& window) {
    if (!m_isAlive || m_body.empty()) {
        return;
    }

    // Khi đang bất tử và ở pha ẩn của hiệu ứng nhấp nháy, giảm độ đậm alpha
    const std::uint8_t alphaValue = (m_isInvincible && !m_isVisible) ? 80 : 255;

    for (std::size_t i = 0; i < m_body.size(); ++i) {
        sf::RectangleShape segmentShape(sf::Vector2f(
            static_cast<float>(config::TILE_SIZE - 2),
            static_cast<float>(config::TILE_SIZE - 2)
        ));

        const sf::Vector2f pixelPos = config::gridToPixel(m_body[i]);
        segmentShape.setPosition(pixelPos.x + 1.0f, pixelPos.y + 1.0f);

        if (i == 0) {
            // Màu đầu rắn: Xanh lá đậm nổi bật (hoặc vàng sáng khi đủ 100% năng lượng)
            if (canBlast()) {
                segmentShape.setFillColor(sf::Color(255, 215, 0, alphaValue));
            } else {
                segmentShape.setFillColor(sf::Color(0, 220, 90, alphaValue));
            }
            segmentShape.setOutlineThickness(2.0f);
            segmentShape.setOutlineColor(sf::Color(255, 255, 255, alphaValue));
        } else {
            // Màu các đốt thân rắn: Xanh lá dịu hơn
            segmentShape.setFillColor(sf::Color(40, 175, 80, alphaValue));
            segmentShape.setOutlineThickness(1.0f);
            segmentShape.setOutlineColor(sf::Color(20, 100, 45, alphaValue));
        }

        window.draw(segmentShape);
    }
}

void Snake::grow(int segments) {
    if (segments <= 0) {
        return;
    }

    // Tính số đốt tối đa còn có thể tăng thêm để không vượt quá MAX_SNAKE_LENGTH (12 đốt)
    const int currentTotal = static_cast<int>(m_body.size()) + m_pendingGrowth;
    const int availableSlots = config::MAX_SNAKE_LENGTH - currentTotal;

    if (availableSlots <= 0) {
        return;
    }

    const int actualGrowth = std::min(segments, availableSlots);

    // Tăng trực tiếp đốt vào đuôi rắn nếu có từ 1 đốt trở lên để chiều dài cập nhật tức thì
    for (int i = 0; i < actualGrowth; ++i) {
        if (!m_body.empty() && static_cast<int>(m_body.size()) < config::MAX_SNAKE_LENGTH) {
            m_body.push_back(m_body.back());
        }
    }
}

bool Snake::isOccupying(const sf::Vector2i& gridPosition) const {
    for (const auto& segment : m_body) {
        if (segment == gridPosition) {
            return true;
        }
    }
    return false;
}

bool Snake::isBodyOccupying(const sf::Vector2i& gridPosition) const {
    for (std::size_t i = 1; i < m_body.size(); ++i) {
        if (m_body[i] == gridPosition) {
            return true;
        }
    }
    return false;
}

int Snake::getDistanceToHead(const sf::Vector2i& gridPosition) const {
    if (m_body.empty()) {
        return 999;
    }
    const int dx = std::abs(gridPosition.x - m_body.front().x);
    const int dy = std::abs(gridPosition.y - m_body.front().y);
    return std::max(dx, dy);
}

int Snake::getMinDistanceToBody(const sf::Vector2i& gridPosition) const {
    int minDistance = 999;
    for (const auto& segment : m_body) {
        const int dx = std::abs(gridPosition.x - segment.x);
        const int dy = std::abs(gridPosition.y - segment.y);
        const int chebyshevDist = std::max(dx, dy);
        if (chebyshevDist < minDistance) {
            minDistance = chebyshevDist;
        }
    }
    return minDistance;
}

bool Snake::checkSelfCollision() const {
    if (m_body.size() < 2) {
        return false;
    }
    return isBodyOccupying(m_body.front());
}

bool Snake::checkWallCollision(const std::vector<sf::Vector2i>& innerWalls) const {
    if (m_body.empty()) {
        return false;
    }

    const sf::Vector2i& head = m_body.front();

    // 1. Kiểm tra chạm tường viền ngoài cố định
    if (config::isBorderWall(head)) {
        return true;
    }

    // 2. Kiểm tra chạm các ô tường bên trong có thể phá hủy
    for (const auto& wallPos : innerWalls) {
        if (head == wallPos) {
            return true;
        }
    }

    return false;
}

bool Snake::handleCollisions(const std::vector<sf::Vector2i>& innerWalls,
                             const std::vector<sf::Vector2i>& bossTiles) {
    if (!m_isAlive || m_body.empty()) {
        return false;
    }

    const sf::Vector2i& head = m_body.front();
    const bool hitBorder = config::isBorderWall(head);
    const bool hitWallOrSelf = checkWallCollision(innerWalls) || checkSelfCollision();

    bool hitBoss = false;
    for (const auto& bossPos : bossTiles) {
        if (isOccupying(bossPos)) {
            hitBoss = true;
            break;
        }
    }

    if (!hitWallOrSelf && !hitBoss) {
        return false;
    }

    // Nếu rắn đang trong thời gian bất tử (1,5 giây):
    // - Không bị trừ mạng.
    // - Tuy nhiên nếu đi xuyên vào tường viền ngoài thì cần đưa rắn về vị trí an toàn để không vượt khỏi bản đồ.
    if (m_isInvincible) {
        if (hitBorder) {
            respawnAtSafePosition(innerWalls, bossTiles);
        }
        return false;
    }

    // Nếu không bất tử thì trừ 1 mạng, đưa về vị trí an toàn và bật bất tử 1,5 giây
    return takeDamage(1, innerWalls, bossTiles);
}

bool Snake::takeDamage(int damageAmount,
                       const std::vector<sf::Vector2i>& innerWalls,
                       const std::vector<sf::Vector2i>& bossTiles) {
    if (!m_isAlive || m_isInvincible || damageAmount <= 0) {
        return false;
    }

    m_lives = std::max(0, m_lives - damageAmount);

    if (m_lives <= 0) {
        m_isAlive = false;
        m_isActive = false;
        return true;
    }

    // Khi mất mạng nhưng vẫn còn mạng: đưa rắn về vị trí an toàn và bất tử 1,5 giây
    respawnAtSafePosition(innerWalls, bossTiles);
    activateInvincibility(config::INVINCIBLE_DURATION);
    return true;
}

void Snake::addLife(int amount) {
    if (!m_isAlive || amount <= 0) {
        return;
    }
    // Hồi mạng nhưng không vượt quá 3 mạng (MAX_LIVES)
    m_lives = std::min(config::MAX_LIVES, m_lives + amount);
}

void Snake::respawnAtSafePosition(const std::vector<sf::Vector2i>& innerWalls,
                                  const std::vector<sf::Vector2i>& bossTiles) {
    const int currentLength = std::clamp(
        static_cast<int>(m_body.size()),
        config::INITIAL_SNAKE_LENGTH,
        config::MAX_SNAKE_LENGTH
    );

    // Hàm cục bộ kiểm tra một ô có bị tường hoặc Boss chiếm chỗ không
    auto isTileBlocked = [&](const sf::Vector2i& pos) -> bool {
        if (!config::isInsideInnerMap(pos)) {
            return true;
        }
        for (const auto& wall : innerWalls) {
            if (wall == pos) {
                return true;
            }
        }
        for (const auto& boss : bossTiles) {
            // Giữ khoảng cách an toàn tối thiểu 2 ô với Boss khi hồi sinh
            if (std::abs(boss.x - pos.x) <= 2 && std::abs(boss.y - pos.y) <= 2) {
                return true;
            }
        }
        return false;
    };

    // Tìm một hàng ngang an toàn trên bản đồ để đặt đầu rắn và ít nhất 3 ô trống phía trước
    sf::Vector2i chosenHead(config::SAFE_SPAWN_X, config::SAFE_SPAWN_Y);
    bool foundSafeLine = false;

    // Ưu tiên kiểm tra quanh tâm bản đồ trước, sau đó quét các hàng
    const std::vector<int> candidateRows = {9, 8, 10, 7, 11, 6, 12, 5, 13, 4, 14, 3, 15, 2, 16, 1};

    for (int row : candidateRows) {
        if (row < 1 || row >= config::MAP_HEIGHT - 1) {
            continue;
        }
        for (int headX = 5; headX <= config::MAP_WIDTH - 5; ++headX) {
            bool lineClear = true;
            // Kiểm tra ô đầu rắn, 2 ô phía trước đầu rắn (để người chơi kịp phản xạ) và các ô thân phía sau
            for (int offsetX = -3; offsetX <= 2; ++offsetX) {
                sf::Vector2i testPos(headX + offsetX, row);
                if (isTileBlocked(testPos)) {
                    lineClear = false;
                    break;
                }
            }
            if (lineClear) {
                chosenHead = sf::Vector2i(headX, row);
                foundSafeLine = true;
                break;
            }
        }
        if (foundSafeLine) {
            break;
        }
    }

    // Xếp lại thân rắn tại vị trí an toàn vừa tìm được, giữ nguyên chiều dài hiện tại
    m_body.clear();
    for (int i = 0; i < currentLength; ++i) {
        int segX = std::max(1, chosenHead.x - i);
        m_body.emplace_back(segX, chosenHead.y);
    }

    m_direction = Direction::RIGHT;
    m_nextDirection = Direction::RIGHT;
    m_moveTimer = 0.0f;
    setGridPosition(m_body.front());
}

void Snake::activateInvincibility(float duration) {
    if (duration <= 0.0f) {
        return;
    }
    m_isInvincible = true;
    m_invincibleTimer = duration;
    m_blinkTimer = 0.0f;
    m_isVisible = false; // Bắt đầu nhấp nháy ngay lập tức để người chơi nhận biết
}

void Snake::addEnergy(int amount) {
    if (amount <= 0) {
        return;
    }
    m_energy = std::min(config::MAX_ENERGY, m_energy + amount);
}

bool Snake::canBlast() const {
    return m_isAlive && (m_energy >= config::MAX_ENERGY);
}

bool Snake::consumeEnergy() {
    if (!canBlast()) {
        return false;
    }
    m_energy = 0;
    return true;
}

void Snake::initBossPhaseAmmo() {
    m_ammo = std::min(config::MAX_AMMO, config::INITIAL_BOSS_PHASE_AMMO);
}

void Snake::addAmmo(int amount) {
    if (amount <= 0) {
        return;
    }
    m_ammo = std::min(config::MAX_AMMO, m_ammo + amount);
}

bool Snake::hasAmmo() const {
    return m_ammo > 0;
}

bool Snake::canShoot() const {
    return m_isAlive && hasAmmo();
}

bool Snake::consumeAmmo(int amount) {
    if (amount <= 0 || m_ammo < amount) {
        return false;
    }
    m_ammo -= amount;
    return true;
}

void Snake::incrementAppleCount() {
    ++m_appleCount;
}

bool Snake::hasReachedBossPhaseThreshold() const {
    return m_appleCount >= config::APPLES_TO_BOSS_PHASE;
}

void Snake::setSpeed(float speed) {
    if (speed > 0.0f) {
        m_speed = speed;
    }
}

bool Snake::isOppositeDirection(Direction currentDir, Direction newDir) {
    return (currentDir == Direction::UP && newDir == Direction::DOWN) ||
           (currentDir == Direction::DOWN && newDir == Direction::UP) ||
           (currentDir == Direction::LEFT && newDir == Direction::RIGHT) ||
           (currentDir == Direction::RIGHT && newDir == Direction::LEFT);
}

sf::Vector2i Snake::getDirectionOffset(Direction dir) {
    switch (dir) {
        case Direction::UP:
            return sf::Vector2i(0, -1);
        case Direction::DOWN:
            return sf::Vector2i(0, 1);
        case Direction::LEFT:
            return sf::Vector2i(-1, 0);
        case Direction::RIGHT:
            return sf::Vector2i(1, 0);
        default:
            return sf::Vector2i(1, 0);
    }
}
