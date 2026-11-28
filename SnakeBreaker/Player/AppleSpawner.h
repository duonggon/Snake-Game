#pragma once
#include <memory>
#include <random>
#include <vector>
#include "Apple.h"
#include "Snake.h"

// ============================================================================
// File: Player/AppleSpawner.h
// Phụ trách: Thành viên 2 – Đào Duy Khôi (Snake & Apple System)
// Mô tả: Lớp AppleSpawner chịu trách nhiệm:
//        - Chọn loại táo ngẫu nhiên theo đúng phân bố xác suất:
//          + NormalApple (Táo đỏ): 85%
//          + EnergyApple (Táo xanh dương): 10%
//          + HealthApple (Táo vàng): 5%
//        - Tìm vị trí xuất hiện hợp lệ trên lưới 24x18:
//          + Không trùng tường viền ngoài & tường bên trong.
//          + Không trùng thân rắn.
//          + Không trùng Boss (2x2 ô).
//          + Không trùng vật phẩm khác.
// ============================================================================

class AppleSpawner {
public:
    AppleSpawner();
    explicit AppleSpawner(unsigned int seed);

    // Sinh ngẫu nhiên một quả táo tại vị trí hợp lệ trên bản đồ
    std::unique_ptr<Apple> spawnApple(
        const Snake& snake,
        const std::vector<sf::Vector2i>& wallPositions = {},
        const std::vector<sf::Vector2i>& bossPositions = {},
        const std::vector<sf::Vector2i>& otherItemPositions = {}
    );

    // Sinh một loại táo chỉ định tại vị trí chỉ định (hỗ trợ kiểm thử từng loại táo)
    std::unique_ptr<Apple> createAppleByType(AppleType type, const sf::Vector2i& gridPosition) const;

    // Quay số ngẫu nhiên chọn loại táo theo tỷ lệ 85% - 10% - 5%
    AppleType rollAppleType();

    // Kiểm tra một tọa độ lưới có hợp lệ để đặt táo hay không
    static bool isPositionValid(
        const sf::Vector2i& candidatePos,
        const Snake& snake,
        const std::vector<sf::Vector2i>& wallPositions = {},
        const std::vector<sf::Vector2i>& bossPositions = {},
        const std::vector<sf::Vector2i>& otherItemPositions = {}
    );

private:
    std::mt19937 m_rng; // Bộ sinh số ngẫu nhiên Mersenne Twister chuẩn C++17
};
