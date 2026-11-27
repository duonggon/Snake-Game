#include "Map.h"
#include "Config.h"
#include <algorithm>
#include <cmath>

Map::Map() : Map(std::random_device{}()) {}
Map::Map(unsigned int seed) : m_random(seed) { reset(); }

void Map::reset() {
    m_walls.clear();
    m_pendingWall.reset();
    m_spawnTimer = 0.0f;
    m_warningTimer = 0.0f;
    m_isSpawning = true;
    createBorderWalls();
}

void Map::createBorderWalls() {
    for (int y = 0; y < config::MAP_HEIGHT; ++y) {
        for (int x = 0; x < config::MAP_WIDTH; ++x) {
            if (x == 0 || y == 0 || x == config::MAP_WIDTH - 1 || y == config::MAP_HEIGHT - 1) {
                m_walls.emplace_back(sf::Vector2i{x, y}, false);
            }
        }
    }
}

bool Map::isInside(sf::Vector2i gridPosition) const {
    return gridPosition.x >= 0 && gridPosition.x < config::MAP_WIDTH &&
           gridPosition.y >= 0 && gridPosition.y < config::MAP_HEIGHT;
}

bool Map::isBlocked(sf::Vector2i gridPosition) const {
    if (!isInside(gridPosition)) return true;
    return std::any_of(m_walls.begin(), m_walls.end(), [&](const Wall& wall) {
        return wall.getGridPosition() == gridPosition;
    });
}

bool Map::isSpawnPositionValid(sf::Vector2i gridPosition,
                              const std::vector<sf::Vector2i>& snakeCells,
                              const std::vector<sf::Vector2i>& appleCells) const {
    if (isBlocked(gridPosition)) return false;
    if (std::find(snakeCells.begin(), snakeCells.end(), gridPosition) != snakeCells.end()) return false;
    if (std::find(appleCells.begin(), appleCells.end(), gridPosition) != appleCells.end()) return false;
    // Phần tử đầu tiên của snakeCells phải là đầu rắn.
    if (!snakeCells.empty()) {
        const auto head = snakeCells.front();
        if (std::max(std::abs(gridPosition.x - head.x),
                     std::abs(gridPosition.y - head.y)) <= config::WALL_SAFE_RADIUS) return false;
    }
    return true;
}

std::optional<Map::WallPair> Map::chooseSpawnPair(
    const std::vector<sf::Vector2i>& snakeCells,
    const std::vector<sf::Vector2i>& appleCells) {
    std::vector<WallPair> candidates;
    // Chi xet ben phai va ben duoi: moi cap duoc liet ke dung mot lan.
    const std::array<sf::Vector2i, 2> offsets{{{1, 0}, {0, 1}}};
    for (int y = 1; y < config::MAP_HEIGHT - 1; ++y) {
        for (int x = 1; x < config::MAP_WIDTH - 1; ++x) {
            const sf::Vector2i first{x, y};
            if (!isSpawnPositionValid(first, snakeCells, appleCells)) continue;
            for (const auto offset : offsets) {
                const auto second = first + offset;
                if (isSpawnPositionValid(second, snakeCells, appleCells))
                    candidates.push_back({first, second});
            }
        }
    }
    if (candidates.empty()) return std::nullopt;
    std::uniform_int_distribution<std::size_t> distribution(0, candidates.size() - 1);
    return candidates[distribution(m_random)];
}

float Map::getSpawnInterval(int appleCount) {
    if (appleCount <= 2) return config::WALL_INTERVAL_EARLY;
    if (appleCount <= 5) return config::WALL_INTERVAL_MID;
    if (appleCount <= 8) return config::WALL_INTERVAL_LATE;
    return config::WALL_INTERVAL_FINAL;
}

void Map::update(float gameDeltaTime, int appleCount,
                 const std::vector<sf::Vector2i>& snakeCells,
                 const std::vector<sf::Vector2i>& appleCells) {
    if (!m_isSpawning || gameDeltaTime <= 0.0f) return;
    if (appleCount >= config::APPLES_TO_BOSS) {
        stopSpawning();
        return;
    }
    if (getDestructibleWallCount() + config::WALL_PAIR_SIZE > config::MAX_WALL_COUNT) {
        m_spawnTimer = 0.0f;
        m_warningTimer = 0.0f;
        m_pendingWall.reset();
        return;
    }
    if (m_pendingWall) {
        m_warningTimer += gameDeltaTime;
        if (m_warningTimer >= config::WALL_WARNING_DURATION) {
            // Kiểm tra lại vì rắn hoặc táo có thể đã đi vào ô cảnh báo.
            const auto& pair = *m_pendingWall;
            const bool isPairValid = std::all_of(pair.begin(), pair.end(), [&](auto cell) {
                return isSpawnPositionValid(cell, snakeCells, appleCells);
            });
            // Sinh ca hai hoac huy ca hai, khong tao tuong le.
            if (isPairValid) {
                for (const auto cell : pair) m_walls.emplace_back(cell, true);
            }
            m_pendingWall.reset();
            m_spawnTimer = 0.0f;
            m_warningTimer = 0.0f;
        }
        return;
    }
    m_spawnTimer += gameDeltaTime;
    if (m_spawnTimer >= getSpawnInterval(appleCount) - config::WALL_WARNING_DURATION) {
        m_pendingWall = chooseSpawnPair(snakeCells, appleCells);
        m_warningTimer = 0.0f;
        if (!m_pendingWall) m_spawnTimer = 0.0f;
    }
}

bool Map::destroyWall(sf::Vector2i gridPosition) {
    auto it = std::find_if(m_walls.begin(), m_walls.end(), [&](const Wall& wall) {
        return wall.isDestructible() && wall.getGridPosition() == gridPosition;
    });
    if (it == m_walls.end()) return false;
    m_walls.erase(it);
    return true;
}

void Map::clearDestructibleWalls() {
    m_walls.erase(std::remove_if(m_walls.begin(), m_walls.end(), [](const Wall& wall) {
        return wall.isDestructible();
    }), m_walls.end());
    m_pendingWall.reset();
    m_spawnTimer = 0.0f;
    m_warningTimer = 0.0f;
}

void Map::stopSpawning() {
    m_isSpawning = false;
    m_pendingWall.reset();
    m_spawnTimer = 0.0f;
    m_warningTimer = 0.0f;
}

int Map::getDestructibleWallCount() const {
    return static_cast<int>(std::count_if(m_walls.begin(), m_walls.end(), [](const Wall& wall) {
        return wall.isDestructible();
    }));
}

std::optional<sf::Vector2i> Map::getPendingWall() const {
    if (!m_pendingWall) return std::nullopt;
    return (*m_pendingWall)[0];
}
std::optional<Map::WallPair> Map::getPendingWallPair() const { return m_pendingWall; }
const std::vector<Wall>& Map::getWalls() const { return m_walls; }

void Map::render(sf::RenderWindow& window) const {
    sf::RectangleShape tile({float(config::TILE_SIZE - 1), float(config::TILE_SIZE - 1)});
    for (int y = 0; y < config::MAP_HEIGHT; ++y) {
        for (int x = 0; x < config::MAP_WIDTH; ++x) {
            tile.setPosition({float(x * config::TILE_SIZE), float(y * config::TILE_SIZE)});
            tile.setFillColor((x + y) % 2 == 0 ? sf::Color(174, 209, 65) : sf::Color(149, 190, 49));
            window.draw(tile);
        }
    }
    for (const auto& wall : m_walls) wall.render(window);
    if (m_pendingWall) {
        tile.setFillColor(sf::Color(255, 175, 38, 180));
        for (const auto cell : *m_pendingWall) {
            tile.setPosition({float(cell.x * config::TILE_SIZE), float(cell.y * config::TILE_SIZE)});
            window.draw(tile);
        }
    }
}
