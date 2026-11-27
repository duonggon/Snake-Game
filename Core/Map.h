#pragma once
#include "Wall.h"
#include "Config.h"
#include <array>
#include <optional>
#include <random>
#include <vector>

class Map {
public:
    Map();
    explicit Map(unsigned int seed); // Cho phép kiểm thử với seed cố định.
    void reset();
    void update(float gameDeltaTime, int appleCount,
                const std::vector<sf::Vector2i>& snakeCells,
                const std::vector<sf::Vector2i>& appleCells);
    void render(sf::RenderWindow& window) const;

    bool isInside(sf::Vector2i gridPosition) const;
    bool isBlocked(sf::Vector2i gridPosition) const;
    bool isSpawnPositionValid(sf::Vector2i gridPosition,
                              const std::vector<sf::Vector2i>& snakeCells,
                              const std::vector<sf::Vector2i>& appleCells) const;
    bool destroyWall(sf::Vector2i gridPosition);
    void clearDestructibleWalls();
    void stopSpawning();
    int getDestructibleWallCount() const;
    using WallPair = std::array<sf::Vector2i, config::WALL_PAIR_SIZE>;
    std::optional<sf::Vector2i> getPendingWall() const; // API cu: o dau tien.
    std::optional<WallPair> getPendingWallPair() const;
    static float getSpawnInterval(int appleCount);
    const std::vector<Wall>& getWalls() const;

private:
    std::vector<Wall> m_walls;
    std::optional<WallPair> m_pendingWall;
    std::mt19937 m_random;
    float m_spawnTimer = 0.0f;
    float m_warningTimer = 0.0f;
    bool m_isSpawning = true;

    void createBorderWalls();
    std::optional<WallPair> chooseSpawnPair(
        const std::vector<sf::Vector2i>& snakeCells,
        const std::vector<sf::Vector2i>& appleCells);
};
