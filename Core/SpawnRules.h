#pragma once
#include "Core/Config.h"
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

// =============================================================================
// QUY TẮC SINH TÁO AN TOÀN GẦN RẮN & NHỊP ĐỘ MỌC TƯỜNG (Core/SpawnRules.h)
// Dùng chung cho Sandbox và có thể tích hợp trực tiếp vào module Map / Player:
// 1. Táo KHÔNG BAO GIỜ xuất hiện ở các mép cạnh và 4 góc bản đồ (cách rìa >= 2 ô).
// 2. Không sinh táo ở ngõ cụt bị tường vây kín để rắn ăn xong luôn có lối thoát.
// 3. Ưu tiên sinh táo ngẫu nhiên GẦN ĐẦU RẮN (khoảng cách 2..6 ô).
// 4. Tường ngẫu nhiên mọc nhanh hơn (1.15s) và giảm dần chu kỳ khi ăn thêm táo.
// =============================================================================
namespace SpawnRules {

    // Tính chu kỳ sinh tường (giây) giảm dần theo số lượng táo đã ăn
    inline float computeWallSpawnInterval(int applesEaten) {
        const float reduced = Config::WALL_SPAWN_BASE_INTERVAL -
                              static_cast<float>(std::max(0, applesEaten)) *
                                  Config::WALL_SPAWN_ACCEL_PER_APPLE;
        return std::max(Config::WALL_SPAWN_MIN_INTERVAL, reduced);
    }

    // Kiểm tra tọa độ (col, row) có nằm trong vùng an toàn cách xa mép & góc bản đồ không
    inline bool isInsideSafeInnerMargin(int col, int row) {
        const int margin = Config::APPLE_BORDER_MARGIN; // Mặc định = 2
        return col >= margin && col < (Config::MAP_COLS - margin) &&
               row >= margin && row < (Config::MAP_ROWS - margin);
    }

    // Chọn ngẫu nhiên ô sinh Táo / Vật phẩm: không sát mép, không kẹt tường, ưu tiên gần đầu rắn
    template <typename IsWallFn, typename IsOccupiedFn>
    sf::Vector2i pickNearbySafeItemCell(
        sf::Vector2i snakeHead,
        sf::Vector2i avoidCell,
        std::mt19937& rng,
        IsWallFn&& isWallAt,
        IsOccupiedFn&& isSnakeAt
    ) {
        const auto countAdjacentWalls = [&](int c, int r) {
            int count = 0;
            if (isWallAt(c + 1, r)) ++count;
            if (isWallAt(c - 1, r)) ++count;
            if (isWallAt(c, r + 1)) ++count;
            if (isWallAt(c, r - 1)) ++count;
            return count;
        };

        const auto collectCandidates = [&](int minDist, int maxDist, int maxAdjWalls) {
            std::vector<sf::Vector2i> candidates;
            for (int r = Config::APPLE_BORDER_MARGIN; r < Config::MAP_ROWS - Config::APPLE_BORDER_MARGIN; ++r) {
                for (int c = Config::APPLE_BORDER_MARGIN; c < Config::MAP_COLS - Config::APPLE_BORDER_MARGIN; ++c) {
                    if ((c == avoidCell.x && r == avoidCell.y) || isWallAt(c, r) || isSnakeAt(c, r)) {
                        continue;
                    }
                    const int chebyshevDist = std::max(std::abs(c - snakeHead.x), std::abs(r - snakeHead.y));
                    if (chebyshevDist < minDist || chebyshevDist > maxDist) {
                        continue;
                    }
                    if (countAdjacentWalls(c, r) > maxAdjWalls) {
                        continue;
                    }
                    candidates.push_back({c, r});
                }
            }
            return candidates;
        };

        // Ưu tiên 1: Gần đầu rắn (2..6 ô), thoáng đãng (tối đa 1 tường kề cạnh)
        auto tier1 = collectCandidates(
            Config::APPLE_MIN_DIST_FROM_HEAD,
            Config::APPLE_MAX_DIST_FROM_HEAD,
            Config::APPLE_MAX_ADJACENT_WALLS
        );
        if (!tier1.empty()) {
            std::uniform_int_distribution<std::size_t> pick(0, tier1.size() - 1);
            return tier1[pick(rng)];
        }

        // Ưu tiên 2: Nếu quanh 6 ô bị tường vây kín, nới nhẹ ra 2..9 ô (vẫn cấm sát mép/góc!)
        auto tier2 = collectCandidates(Config::APPLE_MIN_DIST_FROM_HEAD, 9, 2);
        if (!tier2.empty()) {
            std::uniform_int_distribution<std::size_t> pick(0, tier2.size() - 1);
            return tier2[pick(rng)];
        }

        // Dự phòng cuối: Bất kỳ ô trống nào trong vùng an toàn cách mép >= 2 ô
        auto tier3 = collectCandidates(1, Config::MAP_COLS, 3);
        if (!tier3.empty()) {
            std::uniform_int_distribution<std::size_t> pick(0, tier3.size() - 1);
            return tier3[pick(rng)];
        }

        return {Config::MAP_COLS / 2, Config::MAP_ROWS / 2};
    }

} // namespace SpawnRules
