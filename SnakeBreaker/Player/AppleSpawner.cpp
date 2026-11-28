#include "AppleSpawner.h"
#include "NormalApple.h"
#include "HealthApple.h"
#include "EnergyApple.h"

// ============================================================================
// File: Player/AppleSpawner.cpp
// Phụ trách: Thành viên 2 – Đào Duy Khôi (Snake & Apple System)
// Mô tả: Cài đặt bộ sinh táo ngẫu nhiên tại vị trí hợp lệ AppleSpawner.
// ============================================================================

AppleSpawner::AppleSpawner()
    : m_rng(std::random_device{}()) {}

AppleSpawner::AppleSpawner(unsigned int seed)
    : m_rng(seed) {}

std::unique_ptr<Apple> AppleSpawner::spawnApple(
    const Snake& snake,
    const std::vector<sf::Vector2i>& wallPositions,
    const std::vector<sf::Vector2i>& bossPositions,
    const std::vector<sf::Vector2i>& otherItemPositions) {

    // Thu thập tất cả các ô trống hợp lệ bên trong bản đồ (x: 1..22, y: 1..16)
    std::vector<sf::Vector2i> validPositions;
    validPositions.reserve((config::MAP_WIDTH - 2) * (config::MAP_HEIGHT - 2));

    for (int y = 1; y < config::MAP_HEIGHT - 1; ++y) {
        for (int x = 1; x < config::MAP_WIDTH - 1; ++x) {
            sf::Vector2i candidatePos(x, y);
            if (isPositionValid(candidatePos, snake, wallPositions, bossPositions, otherItemPositions)) {
                validPositions.push_back(candidatePos);
            }
        }
    }

    // Trường hợp bản đồ không còn bất kỳ ô trống nào
    if (validPositions.empty()) {
        return nullptr;
    }

    // Chọn ngẫu nhiên 1 ô trong danh sách các ô hợp lệ (đảm bảo luôn tìm được trong O(N) mà không bị lặp vô hạn)
    std::uniform_int_distribution<std::size_t> posDist(0, validPositions.size() - 1);
    const sf::Vector2i chosenPos = validPositions[posDist(m_rng)];

    // Chọn loại táo theo xác suất 85% Đỏ - 10% Xanh dương - 5% Vàng
    const AppleType chosenType = rollAppleType();
    return createAppleByType(chosenType, chosenPos);
}

std::unique_ptr<Apple> AppleSpawner::createAppleByType(AppleType type, const sf::Vector2i& gridPosition) const {
    switch (type) {
        case AppleType::NORMAL_APPLE:
            return std::make_unique<NormalApple>(gridPosition);
        case AppleType::ENERGY_APPLE:
            return std::make_unique<EnergyApple>(gridPosition);
        case AppleType::HEALTH_APPLE:
            return std::make_unique<HealthApple>(gridPosition);
        default:
            return std::make_unique<NormalApple>(gridPosition);
    }
}

AppleType AppleSpawner::rollAppleType() {
    // Sinh số ngẫu nhiên từ 1 đến 100
    std::uniform_int_distribution<int> rateDist(1, 100);
    const int roll = rateDist(m_rng);

    // 1..85 (85%): NormalApple (Táo đỏ)
    if (roll <= config::NORMAL_APPLE_RATE) {
        return AppleType::NORMAL_APPLE;
    }
    // 86..95 (10%): EnergyApple (Táo xanh dương)
    if (roll <= config::NORMAL_APPLE_RATE + config::ENERGY_APPLE_RATE) {
        return AppleType::ENERGY_APPLE;
    }
    // 96..100 (5%): HealthApple (Táo vàng)
    return AppleType::HEALTH_APPLE;
}

bool AppleSpawner::isPositionValid(
    const sf::Vector2i& candidatePos,
    const Snake& snake,
    const std::vector<sf::Vector2i>& wallPositions,
    const std::vector<sf::Vector2i>& bossPositions,
    const std::vector<sf::Vector2i>& otherItemPositions) {

    // 1. Không nằm trên tường viền ngoài hoặc ngoài bản đồ
    if (!config::isInsideInnerMap(candidatePos)) {
        return false;
    }

    // 2. Không trùng với đầu hoặc thân rắn
    if (snake.isOccupying(candidatePos)) {
        return false;
    }

    // 3. Không trùng với bất kỳ ô tường bên trong nào
    for (const auto& wallPos : wallPositions) {
        if (wallPos == candidatePos) {
            return false;
        }
    }

    // 4. Không trùng với vùng 2x2 của Boss
    for (const auto& bossPos : bossPositions) {
        if (bossPos == candidatePos) {
            return false;
        }
    }

    // 5. Không trùng với vật phẩm / quả táo khác đang tồn tại
    for (const auto& itemPos : otherItemPositions) {
        if (itemPos == candidatePos) {
            return false;
        }
    }

    return true;
}
