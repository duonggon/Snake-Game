#include "Core/Map.h"
#include "Core/Config.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
    void require(bool condition, const char* message) {
        if (!condition) throw std::runtime_error(message);
    }
    bool areAdjacent(const Map::WallPair& pair) {
        return std::abs(pair[0].x - pair[1].x) + std::abs(pair[0].y - pair[1].y) == 1;
    }
}

int main() {
    try {
        Map map(42);
        const std::vector<sf::Vector2i> snakeCells{{6, 6}, {5, 6}, {4, 6}};
        const std::vector<sf::Vector2i> appleCells{{15, 10}};
        require(map.getWalls().size() == 80, "Border must have 80 walls");
        require(map.isBlocked({0, 0}) && map.isBlocked({-1, 3}), "Border/outside blocked");
        require(!map.destroyWall({0, 0}), "Border cannot be destroyed");
        require(!map.isSpawnPositionValid({8, 8}, snakeCells, appleCells), "Head safety radius");
        require(!map.isSpawnPositionValid({15, 10}, snakeCells, appleCells), "Apple blocked");
        require(!map.isSpawnPositionValid({4, 6}, snakeCells, appleCells), "Snake blocked");
        require(Map::getSpawnInterval(0) == 2.0f && Map::getSpawnInterval(3) == 1.5f &&
                Map::getSpawnInterval(6) == 1.0f && Map::getSpawnInterval(9) == 0.7f, "Intervals");
        const float earlyWarningAt = Map::getSpawnInterval(0) - config::WALL_WARNING_DURATION;
        map.update(earlyWarningAt / 2, 0, snakeCells, appleCells);
        require(!map.getPendingWallPair(), "Warning not early");
        map.update(earlyWarningAt / 2 + 0.001f, 0, snakeCells, appleCells);
        const auto pending = map.getPendingWallPair();
        require(pending && areAdjacent(*pending), "Warning must contain adjacent pair");
        require(map.getPendingWall() == (*pending)[0], "Legacy warning API");
        for (auto cell : *pending) {
            require(!map.isBlocked(cell), "Warning not solid");
            require(map.isSpawnPositionValid(cell, snakeCells, appleCells), "Warning position safe");
        }
        map.update(config::WALL_WARNING_DURATION, 0, snakeCells, {(*pending)[1]});
        require(map.getDestructibleWallCount() == 0, "Cancel whole pair if second cell occupied");
        map.update(earlyWarningAt, 0, snakeCells, appleCells);
        const auto next = map.getPendingWallPair();
        require(next && areAdjacent(*next), "Warning pair exists");
        map.update(config::WALL_WARNING_DURATION, 0, snakeCells, appleCells);
        require(map.isBlocked((*next)[0]) && map.isBlocked((*next)[1]) &&
                map.getDestructibleWallCount() == 2, "Both walls appear together");
        require(map.destroyWall((*next)[0]) && !map.isBlocked((*next)[0]) &&
                map.isBlocked((*next)[1]), "Destroy internal wall independently");
        require(map.destroyWall((*next)[1]), "Destroy second wall");
        const float lateWarningAt = Map::getSpawnInterval(9) - config::WALL_WARNING_DURATION;
        for (int i = 0; i < 50; ++i) {
            const int before = map.getDestructibleWallCount();
            map.update(lateWarningAt, 9, snakeCells, appleCells);
            const auto pair = map.getPendingWallPair();
            if (pair) require(areAdjacent(*pair), "All pairs adjacent");
            map.update(config::WALL_WARNING_DURATION, 9, snakeCells, appleCells);
            const int delta = map.getDestructibleWallCount() - before;
            require(delta == 0 || delta == 2, "Never spawn single walls");
        }
        require(map.getDestructibleWallCount() == config::MAX_WALL_COUNT, "Wall cap");
        for (const auto& wall : map.getWalls()) {
            if (!wall.isDestructible()) continue;
            const auto cell = wall.getGridPosition();
            require(cell != appleCells[0] && cell != snakeCells[0] && cell != snakeCells[1] &&
                    cell != snakeCells[2], "Never spawn over snake/apple");
        }
        // Neu da pha 1 o, khong vuot tran de sinh cap moi.
        const auto removeCell = map.getWalls().back().getGridPosition();
        require(map.destroyWall(removeCell), "Destroy at cap");
        map.update(10, 9, snakeCells, appleCells);
        require(!map.getPendingWallPair() && map.getDestructibleWallCount() == config::MAX_WALL_COUNT - 1,
                "No partial pair at odd remaining capacity");
        map.stopSpawning();
        map.clearDestructibleWalls();
        map.update(10.0f, 10, snakeCells, appleCells);
        require(map.getWalls().size() == 80 && !map.getPendingWallPair(), "Boss transition cleanup");
        map.reset();
        map.update(earlyWarningAt, 0, snakeCells, appleCells);
        require(map.getPendingWallPair().has_value(), "Reset enables spawning");
        std::cout << "Core map tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
