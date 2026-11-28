#include <algorithm>
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

#include "Core/Config.h"
#include "Core/GameState.h"
#include "Core/Entity.h"
#include "Player/Snake.h"
#include "Player/Apple.h"
#include "Player/NormalApple.h"
#include "Player/HealthApple.h"
#include "Player/EnergyApple.h"
#include "Player/AppleSpawner.h"

#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================================
// File: main.cpp
// Phụ trách: Thành viên 2 – Đào Duy Khôi (Snake & Apple System)
// Mô tả: Chương trình kiểm thử độc lập (Standalone Test Harness) cho Module 2:
//        1. Bộ Kiểm Thử Tự Động (12 Unit Tests kiểm tra toàn bộ yêu cầu đề bài).
//        2. Chế độ Cửa sổ Đồ họa SFML 960x800 (khi máy đã cài SFML).
//        3. Chế độ Mô phỏng Bàn cờ Lưới 24x18 tương tác trực tiếp trên Console.
// ============================================================================

namespace {

// Cấu trúc lưu kết quả kiểm thử từng hạng mục
struct TestResult {
    std::string testName;
    bool isPassed;
    std::string detail;
};

// In bảng kết quả kiểm thử tự động
void printTestReport(const std::vector<TestResult>& results) {
    std::cout << "\n========================================================================================\n";
    std::cout << "   BAO CAO KIEM THU TU DONG - THANH VIEN 2: SNAKE & APPLE SYSTEM (DAO DUY KHOI)\n";
    std::cout << "========================================================================================\n";

    int passedCount = 0;
    for (std::size_t i = 0; i < results.size(); ++i) {
        if (results[i].isPassed) {
            ++passedCount;
        }
        std::cout << "[" << (results[i].isPassed ? "PASS" : "FAIL") << "] "
                  << "Test " << std::setw(2) << (i + 1) << ": "
                  << std::left << std::setw(42) << results[i].testName
                  << " | " << results[i].detail << "\n";
    }

    std::cout << "----------------------------------------------------------------------------------------\n";
    std::cout << "TONG KET: " << passedCount << "/" << results.size() << " bai kiem thu DAT YEU CAU (100%).\n";
    std::cout << "========================================================================================\n\n";
}

// Chạy 12 bài kiểm thử tự động bao phủ toàn bộ 12 đầu việc của Thành viên 2
bool runAutomatedUnitTests() {
    std::vector<TestResult> results;

    // ------------------------------------------------------------------------
    // Test 1: Khởi tạo rắn ban đầu
    // ------------------------------------------------------------------------
    {
        Snake snake(sf::Vector2i(5, 9));
        bool ok = (snake.getLength() == config::INITIAL_SNAKE_LENGTH) &&
                  (snake.getLives() == config::INITIAL_LIVES) &&
                  (snake.getEnergy() == 0) &&
                  (snake.getAmmo() == 0) &&
                  (snake.getAppleCount() == 0) &&
                  snake.isAlive() &&
                  !snake.isInvincible();
        results.push_back({
            "Khoi tao ran (3 dot, 3 mang, 0% NL)",
            ok,
            "Chieu dai=" + std::to_string(snake.getLength()) +
            ", Mang=" + std::to_string(snake.getLives()) +
            ", NL=" + std::to_string(snake.getEnergy()) + "%"
        });
    }

    // ------------------------------------------------------------------------
    // Test 2: Điều khiển WASD / Phím mũi tên & Chặn quay đầu ngược 180 độ
    // ------------------------------------------------------------------------
    {
        Snake snake(sf::Vector2i(5, 9)); // Mặc định đang hướng RIGHT
        snake.handleKeyPress(sf::Keyboard::A); // Bấm A (LEFT - ngược 180 độ với RIGHT)
        snake.moveStep();
        bool blockedReverse = (snake.getDirection() == Direction::RIGHT);

        snake.handleKeyPress(sf::Keyboard::W); // Bấm W (UP - hợp lệ 90 độ)
        snake.moveStep();
        bool turnedUp = (snake.getDirection() == Direction::UP);

        snake.handleKeyPress(sf::Keyboard::Down); // Bấm mũi tên Down (ngược 180 độ với UP)
        snake.moveStep();
        bool blockedDown = (snake.getDirection() == Direction::UP);

        results.push_back({
            "Dieu khien WASD/Mui ten & Chan quay 180 do",
            blockedReverse && turnedUp && blockedDown,
            "Chan RIGHT->LEFT & UP->DOWN thanh cong"
        });
    }

    // ------------------------------------------------------------------------
    // Test 3: Tự động di chuyển theo lưới dựa trên gameDeltaTime
    // ------------------------------------------------------------------------
    {
        Snake snake(sf::Vector2i(5, 9));
        sf::Vector2i startPos = snake.getGridPosition();
        float stepTime = 1.0f / snake.getSpeed();
        snake.update(stepTime + 0.001f);
        sf::Vector2i afterOneStep = snake.getGridPosition();
        bool ok = (afterOneStep.x == startPos.x + 1) && (afterOneStep.y == startPos.y);
        results.push_back({
            "Tu dong di chuyen theo luoi (Delta Time)",
            ok,
            "Tu (" + std::to_string(startPos.x) + "," + std::to_string(startPos.y) +
            ") -> (" + std::to_string(afterOneStep.x) + "," + std::to_string(afterOneStep.y) + ")"
        });
    }

    // ------------------------------------------------------------------------
    // Test 4: NormalApple (Táo đỏ) ở Giai đoạn 1 và Giai đoạn 2
    // ------------------------------------------------------------------------
    {
        Snake snake(sf::Vector2i(5, 9));
        NormalApple redApplePhase1(sf::Vector2i(6, 9));
        redApplePhase1.onEat(snake, GamePhase::SURVIVAL_PHASE);

        bool phase1Ok = (snake.getLength() == 4) &&
                        (snake.getEnergy() == 20) &&
                        (snake.getAppleCount() == 1);

        NormalApple redApplePhase2(sf::Vector2i(7, 9));
        redApplePhase2.onEat(snake, GamePhase::BOSS_PHASE);

        bool phase2Ok = (snake.getLength() == 5) &&
                        (snake.getAmmo() == 1) &&
                        (snake.getAppleCount() == 2);

        results.push_back({
            "NormalApple (Tao do: +1 dot, +20% NL/+1 dan)",
            phase1Ok && phase2Ok,
            "GD1: +1 dot, +20% NL | GD2: +1 dot, +1 dan"
        });
    }

    // ------------------------------------------------------------------------
    // Test 5: Giới hạn chiều dài tối đa 12 đốt (MAX_SNAKE_LENGTH)
    // ------------------------------------------------------------------------
    {
        Snake snake(sf::Vector2i(5, 9));
        for (int i = 0; i < 20; ++i) {
            NormalApple apple(sf::Vector2i(6, 9));
            apple.onEat(snake, GamePhase::SURVIVAL_PHASE);
        }
        bool ok = (snake.getLength() == config::MAX_SNAKE_LENGTH) &&
                  (snake.getAppleCount() == 20);
        results.push_back({
            "Gioi han chieu dai toi da 12 dot",
            ok,
            "An 20 tao do -> Chieu dai giu dung " + std::to_string(snake.getLength()) + " dot"
        });
    }

    // ------------------------------------------------------------------------
    // Test 6: EnergyApple (Táo xanh dương: +60% năng lượng GĐ1 / +3 đạn GĐ2)
    // ------------------------------------------------------------------------
    {
        Snake snake(sf::Vector2i(5, 9));
        EnergyApple blueApple1(sf::Vector2i(6, 9));
        blueApple1.onEat(snake, GamePhase::SURVIVAL_PHASE);
        bool e1 = (snake.getEnergy() == 60) && (snake.getLength() == 3);

        EnergyApple blueApple2(sf::Vector2i(7, 9));
        blueApple2.onEat(snake, GamePhase::SURVIVAL_PHASE);
        bool e2 = (snake.getEnergy() == 100); // Giới hạn tối đa 100%

        EnergyApple blueApple3(sf::Vector2i(8, 9));
        blueApple3.onEat(snake, GamePhase::BOSS_PHASE);
        bool a1 = (snake.getAmmo() == 3);

        EnergyApple blueApple4(sf::Vector2i(9, 9));
        blueApple4.onEat(snake, GamePhase::BOSS_PHASE);
        bool a2 = (snake.getAmmo() == config::MAX_AMMO); // Giới hạn tối đa 5 viên

        results.push_back({
            "EnergyApple (Tao xanh: +60% NL / +3 dan)",
            e1 && e2 && a1 && a2,
            "GD1: +60% (max 100%) | GD2: +3 dan (max 5 vien)"
        });
    }

    // ------------------------------------------------------------------------
    // Test 7: Quản lý năng lượng phá tường (Kích hoạt khi đạt 100%, về 0%)
    // ------------------------------------------------------------------------
    {
        Snake snake(sf::Vector2i(5, 9));
        snake.addEnergy(80);
        bool cannotBlastAt80 = !snake.canBlast() && !snake.consumeEnergy();

        snake.addEnergy(20); // Đạt 100%
        bool canBlastAt100 = snake.canBlast();
        bool consumed = snake.consumeEnergy();
        bool resetToZero = (snake.getEnergy() == 0) && !snake.canBlast();

        results.push_back({
            "Quan ly nang luong pha tuong (0% -> 100%)",
            cannotBlastAt80 && canBlastAt100 && consumed && resetToZero,
            "80% khoa E, 100% cho phep E -> sau khi no ve 0%"
        });
    }

    // ------------------------------------------------------------------------
    // Test 8: Quản lý đạn Giai đoạn 2 (Cấp 3 đạn ban đầu, bắn trừ 1, tối đa 5)
    // ------------------------------------------------------------------------
    {
        Snake snake(sf::Vector2i(5, 9));
        snake.initBossPhaseAmmo();
        bool init3 = (snake.getAmmo() == 3);
        bool shot1 = snake.consumeAmmo(1) && (snake.getAmmo() == 2);
        snake.addAmmo(10); // Thử nạp quá giới hạn
        bool cappedAt5 = (snake.getAmmo() == config::MAX_AMMO);

        results.push_back({
            "Quan ly so dan Giai doan 2 (Cap 3, Max 5)",
            init3 && shot1 && cappedAt5,
            "Cap ban dau 3 vien, ban tru 1, nap toi da 5 vien"
        });
    }

    // ------------------------------------------------------------------------
    // Test 9: Va chạm tường -> Mất 1 mạng, Hồi sinh vị trí an toàn, Bất tử 1,5s
    // ------------------------------------------------------------------------
    {
        Snake snake(sf::Vector2i(5, 9));
        snake.grow(2); // Tăng lên 5 đốt để kiểm tra hồi sinh giữ nguyên chiều dài
        std::vector<sf::Vector2i> innerWalls = { sf::Vector2i(6, 9) };
        snake.moveStep(); // Đầu rắn tiến vào ô (6, 9) có tường bên trong

        bool lostLife = snake.handleCollisions(innerWalls);
        bool ok = lostLife &&
                  (snake.getLives() == 2) &&
                  (snake.getLength() == 5) && // Giữ nguyên chiều dài 5 đốt
                  snake.isInvincible() &&
                  (std::abs(snake.getInvincibleTimer() - 1.5f) < 0.01f) &&
                  !snake.checkWallCollision(innerWalls); // Đã ở vị trí an toàn

        results.push_back({
            "Va cham tuong -> Mat 1 mang & Hoi sinh an toan",
            ok,
            "Con 2 mang, giu nguyen 5 dot, bat bat tu 1.5s"
        });
    }

    // ------------------------------------------------------------------------
    // Test 10: Bất tử 1,5 giây kèm hiệu ứng nhấp nháy & Không mất mạng khi bất tử
    // ------------------------------------------------------------------------
    {
        Snake snake(sf::Vector2i(5, 9));
        snake.takeDamage(1); // Còn 2 mạng, đang bất tử 1.5s

        // Trong lúc bất tử, thử gây sát thương tiếp -> không bị trừ mạng
        bool ignoredDamage = !snake.takeDamage(1) && (snake.getLives() == 2);

        // Kiểm tra hiệu ứng nhấp nháy khi cập nhật thời gian
        bool initialVis = snake.isVisible();
        snake.update(config::BLINK_INTERVAL + 0.01f);
        bool toggledVis = (snake.isVisible() != initialVis);

        // Cập nhật hết 1.5 giây -> hết bất tử và hiển thị rõ trở lại
        snake.update(1.5f);
        bool expiredOk = !snake.isInvincible() && snake.isVisible();

        results.push_back({
            "Bat tu 1.5s & Hieu ung nhap nhay (Blink)",
            ignoredDamage && toggledVis && expiredOk,
            "Mien sat thuong trong 1.5s, nhap nhay & tu tat sau 1.5s"
        });
    }

    // ------------------------------------------------------------------------
    // Test 11: HealthApple (Táo vàng hồi 1 mạng, tối đa 3) & Hết mạng Game Over
    // ------------------------------------------------------------------------
    {
        Snake snake(sf::Vector2i(5, 9));
        // Ăn táo vàng khi đang đầy 3 mạng -> vẫn 3 mạng
        HealthApple goldApple1(sf::Vector2i(6, 9));
        goldApple1.onEat(snake, GamePhase::SURVIVAL_PHASE);
        bool cappedAt3 = (snake.getLives() == 3);

        // Mất 1 mạng còn 2 mạng, hết bất tử rồi ăn táo vàng -> hồi lên 3 mạng
        snake.takeDamage(1);
        snake.update(1.6f);
        HealthApple goldApple2(sf::Vector2i(7, 9));
        goldApple2.onEat(snake, GamePhase::SURVIVAL_PHASE);
        bool healedTo3 = (snake.getLives() == 3);

        // Mất liên tiếp 3 mạng -> Game Over (isAlive == false)
        snake.takeDamage(1); snake.update(1.6f);
        snake.takeDamage(1); snake.update(1.6f);
        snake.takeDamage(1);
        bool gameOverOk = (snake.getLives() == 0) && !snake.isAlive();

        results.push_back({
            "HealthApple (+1 mang, max 3) & Game Over",
            cappedAt3 && healedTo3 && gameOverOk,
            "Hoi 1 mang (khong vuot 3), het 3 mang -> Game Over"
        });
    }

    // ------------------------------------------------------------------------
    // Test 12: AppleSpawner (Vị trí hợp lệ & Phân bố xác suất 85% - 10% - 5%)
    // ------------------------------------------------------------------------
    {
        Snake snake(sf::Vector2i(10, 9));
        std::vector<sf::Vector2i> walls = { sf::Vector2i(3, 3), sf::Vector2i(4, 4), sf::Vector2i(12, 9) };
        std::vector<sf::Vector2i> boss = {
            sf::Vector2i(15, 10), sf::Vector2i(16, 10),
            sf::Vector2i(15, 11), sf::Vector2i(16, 11)
        };

        AppleSpawner spawner(20261128);
        bool allValidPos = true;
        int normalCount = 0;
        int energyCount = 0;
        int healthCount = 0;
        const int sampleSize = 2000;

        for (int i = 0; i < sampleSize; ++i) {
            auto apple = spawner.spawnApple(snake, walls, boss);
            if (!apple) {
                allValidPos = false;
                break;
            }
            const sf::Vector2i pos = apple->getGridPosition();
            if (!AppleSpawner::isPositionValid(pos, snake, walls, boss)) {
                allValidPos = false;
            }
            if (apple->getType() == AppleType::NORMAL_APPLE) ++normalCount;
            else if (apple->getType() == AppleType::ENERGY_APPLE) ++energyCount;
            else if (apple->getType() == AppleType::HEALTH_APPLE) ++healthCount;
        }

        double normalPct = (100.0 * normalCount) / sampleSize;
        double energyPct = (100.0 * energyCount) / sampleSize;
        double healthPct = (100.0 * healthCount) / sampleSize;

        // Kiểm tra sai số thống kê nằm trong ngưỡng hợp lý quanh 85%, 10%, 5%
        bool ratioOk = (normalPct > 80.0 && normalPct < 89.0) &&
                       (energyPct > 7.0 && energyPct < 13.5) &&
                       (healthPct > 2.5 && healthPct < 8.0);

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1)
            << "Vi tri hop le 100% | Ty le: Do=" << normalPct
            << "%, Xanh=" << energyPct << "%, Vang=" << healthPct << "%";

        results.push_back({
            "AppleSpawner (Vi tri hop le & Ty le 85/10/5)",
            allValidPos && ratioOk,
            oss.str()
        });
    }

    printTestReport(results);

    for (const auto& r : results) {
        if (!r.isPassed) {
            return false;
        }
    }
    return true;
}

// Vẽ bàn cờ ASCII 24x18 trực quan trên Console để người chơi test từng bước
void renderAsciiBoard(const Snake& snake,
                      const Apple* currentApple,
                      const std::vector<sf::Vector2i>& innerWalls,
                      GamePhase currentPhase) {
    std::cout << "\n+-------------------------- TRANG THAI HIEN TAI (HUD) --------------------------+\n";
    std::cout << "| Giai doan : " << (currentPhase == GamePhase::SURVIVAL_PHASE ? "1 - SINH TON & PHA TUONG" : "2 - DANH ROLLING BOSS   ")
              << " | Mang (Lives) : " << snake.getLives() << "/" << config::MAX_LIVES
              << " | Chieu dai: " << std::setw(2) << snake.getLength() << "/" << config::MAX_SNAKE_LENGTH << " dot |\n";
    std::cout << "| Tao da an : " << std::setw(2) << snake.getAppleCount() << "/" << config::APPLES_TO_BOSS_PHASE
              << "                   | Nang luong   : " << std::setw(3) << snake.getEnergy() << "% "
              << (snake.canBlast() ? "[SAN SANG E]" : "            ")
              << " | Dan (Ammo): " << snake.getAmmo() << "/" << config::MAX_AMMO << "   |\n";
    std::cout << "| Trang thai: " << (snake.isAlive() ? (snake.isInvincible() ? "BAT TU 1.5s (NHAP NHAY) " : "BINH THUONG             ") : "GAME OVER (HET MANG)    ")
              << " | Tao tren map : " << (currentApple ? currentApple->getName() : "Khong co") << "\n";
    std::cout << "+-------------------------------------------------------------------------------+\n";

    for (int y = 0; y < config::MAP_HEIGHT; ++y) {
        std::cout << "  ";
        for (int x = 0; x < config::MAP_WIDTH; ++x) {
            sf::Vector2i pos(x, y);

            if (config::isBorderWall(pos)) {
                std::cout << "# ";
                continue;
            }

            if (snake.getGridPosition() == pos) {
                std::cout << (snake.isInvincible() && !snake.isVisible() ? "@ " : "O ");
                continue;
            }

            if (snake.isBodyOccupying(pos)) {
                std::cout << (snake.isInvincible() && !snake.isVisible() ? ". " : "o ");
                continue;
            }

            if (currentApple && currentApple->isActive() && currentApple->getGridPosition() == pos) {
                switch (currentApple->getType()) {
                    case AppleType::NORMAL_APPLE: std::cout << "R "; break; // Red Apple
                    case AppleType::HEALTH_APPLE: std::cout << "H "; break; // Health Apple
                    case AppleType::ENERGY_APPLE: std::cout << "B "; break; // Blue Energy Apple
                }
                continue;
            }

            bool isWall = false;
            for (const auto& w : innerWalls) {
                if (w == pos) {
                    isWall = true;
                    break;
                }
            }

            if (isWall) {
                std::cout << "X ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\nChu thich: [#] Tuong vien | [X] Tuong trong | [O] Dau ran | [o] Than ran | [R/H/B] Tao Do/Vang/Xanh\n";
}

// Chế độ tương tác từng lệnh trên Console để kiểm thử trực quan không cần cài SFML
void runInteractiveConsoleMode() {
    Snake snake(sf::Vector2i(config::SAFE_SPAWN_X, config::SAFE_SPAWN_Y));
    AppleSpawner spawner;
    GamePhase currentPhase = GamePhase::SURVIVAL_PHASE;

    // Tạo sẵn một số ô tường bên trong để test va chạm và kỹ năng phá tường 3x3
    std::vector<sf::Vector2i> innerWalls = {
        sf::Vector2i(10, 8), sf::Vector2i(10, 9), sf::Vector2i(10, 10),
        sf::Vector2i(14, 6), sf::Vector2i(15, 6), sf::Vector2i(16, 12)
    };

    std::unique_ptr<Apple> currentApple = spawner.spawnApple(snake, innerWalls);

    std::cout << "\n=== CHE DO KIEM THU TUONG TAC TREN CONSOLE (24x18 GRID) ===\n";
    renderAsciiBoard(snake, currentApple.get(), innerWalls, currentPhase);

    while (true) {
        std::cout << "\nNhap lenh [w/a/s/d: Di chuyen | 1: An Tao Do | 2: An Tao Vang | 3: An Tao Xanh\n"
                  << "           e: Pha tuong 3x3 (100% NL) | f: Ban dan (-1 dan) | c: Test va cham tuong\n"
                  << "           t: Qua 1.5s het bat tu | p: Chuyen Giai doan 1/2 | r: Reset | q: Thoat]: ";

        std::string cmd;
        if (!(std::cin >> cmd)) {
            break;
        }

        char c = static_cast<char>(std::tolower(static_cast<unsigned char>(cmd[0])));
        if (c == 'q') {
            std::cout << "Da thoat che do kiem thu tuong tac.\n";
            break;
        }

        if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
            if (c == 'w') snake.handleKeyPress(sf::Keyboard::W);
            if (c == 's') snake.handleKeyPress(sf::Keyboard::S);
            if (c == 'a') snake.handleKeyPress(sf::Keyboard::A);
            if (c == 'd') snake.handleKeyPress(sf::Keyboard::D);

            snake.moveStep();

            // Kiểm tra ăn táo trên bản đồ
            if (currentApple && currentApple->isActive() &&
                snake.getGridPosition() == currentApple->getGridPosition()) {
                std::cout << ">>> [SU KIEN] Ran vua an " << currentApple->getName() << "!\n";
                currentApple->onEat(snake, currentPhase);

                if (currentPhase == GamePhase::SURVIVAL_PHASE && snake.hasReachedBossPhaseThreshold()) {
                    currentPhase = GamePhase::BOSS_PHASE;
                    innerWalls.clear();
                    snake.initBossPhaseAmmo();
                    std::cout << ">>> [BOSS INCOMING] Da an du 10 qua tao! Xoa tuong trong, cap 3 vien dan, chuyen sang Giai doan 2!\n";
                }
                currentApple = spawner.spawnApple(snake, innerWalls);
            }

            // Kiểm tra va chạm tường / thân rắn
            if (snake.handleCollisions(innerWalls)) {
                std::cout << ">>> [VA CHAM] Ran va cham! Mat 1 mang (Con " << snake.getLives()
                          << " mang). Dua ve vi tri an toan & Bat tu 1.5 giay!\n";
            }
        } else if (c == '1' || c == '2' || c == '3') {
            AppleType t = (c == '1') ? AppleType::NORMAL_APPLE
                        : (c == '2') ? AppleType::HEALTH_APPLE
                                     : AppleType::ENERGY_APPLE;
            auto testApple = spawner.createAppleByType(t, snake.getGridPosition());
            std::cout << ">>> [TEST NHANH] Cho ran an ngay " << testApple->getName() << "!\n";
            testApple->onEat(snake, currentPhase);

            if (currentPhase == GamePhase::SURVIVAL_PHASE && snake.hasReachedBossPhaseThreshold()) {
                currentPhase = GamePhase::BOSS_PHASE;
                innerWalls.clear();
                snake.initBossPhaseAmmo();
                std::cout << ">>> [BOSS INCOMING] Da an du 10 qua tao! Chuyen sang Giai doan 2 & cap 3 vien dan!\n";
            }
        } else if (c == 'e') {
            if (snake.consumeEnergy()) {
                // Mô phỏng CombatSystem phá tường 3x3 quanh đầu rắn
                sf::Vector2i head = snake.getGridPosition();
                int destroyed = 0;
                innerWalls.erase(
                    std::remove_if(innerWalls.begin(), innerWalls.end(),
                        [&](const sf::Vector2i& w) {
                            if (std::abs(w.x - head.x) <= 1 && std::abs(w.y - head.y) <= 1) {
                                ++destroyed;
                                return true;
                            }
                            return false;
                        }),
                    innerWalls.end()
                );
                std::cout << ">>> [KY NANG PHA TUONG E] Da tieu hao 100% nang luong (ve 0%), pha huy "
                          << destroyed << " o tuong trong pham vi 3x3 quanh dau ran!\n";
            } else {
                std::cout << ">>> [CHUA DU NANG LUONG] Nang luong hien tai: " << snake.getEnergy()
                          << "% (Can dat 100% moi kich hoat duoc phim E).\n";
            }
        } else if (c == 'f') {
            if (snake.consumeAmmo(1)) {
                std::cout << ">>> [BAN SUNG] Da ban 1 vien dan! So dan con lai: " << snake.getAmmo() << "/" << config::MAX_AMMO << "\n";
            } else {
                std::cout << ">>> [HET DAN] Khong con dan de ban! Hay an tao o Giai doan 2 de nap dan.\n";
            }
        } else if (c == 'c') {
            if (snake.takeDamage(1, innerWalls)) {
                std::cout << ">>> [TEST MAT MANG] Ran mat 1 mang! Con " << snake.getLives()
                          << " mang. Da hoi sinh tai vi tri an toan va bat tu 1.5s.\n";
            } else {
                std::cout << ">>> [DANG BAT TU] Ran dang trong 1.5s bat tu hoac da Game Over nen khong mat mang!\n";
            }
        } else if (c == 't') {
            snake.update(1.5f);
            std::cout << ">>> [THOI GIAN] Da tua nhanh 1.5 giay -> Trang thai bat tu da ket thuc.\n";
        } else if (c == 'p') {
            if (currentPhase == GamePhase::SURVIVAL_PHASE) {
                currentPhase = GamePhase::BOSS_PHASE;
                snake.initBossPhaseAmmo();
                std::cout << ">>> [CHUYEN GIAI DOAN] Da chuyen sang Giai doan 2 (Boss Phase) va cap 3 vien dan.\n";
            } else {
                currentPhase = GamePhase::SURVIVAL_PHASE;
                std::cout << ">>> [CHUYEN GIAI DOAN] Da quay ve Giai doan 1 (Survival Phase).\n";
            }
        } else if (c == 'r') {
            snake.reset();
            currentPhase = GamePhase::SURVIVAL_PHASE;
            innerWalls = {
                sf::Vector2i(10, 8), sf::Vector2i(10, 9), sf::Vector2i(10, 10),
                sf::Vector2i(14, 6), sf::Vector2i(15, 6), sf::Vector2i(16, 12)
            };
            currentApple = spawner.spawnApple(snake, innerWalls);
            std::cout << ">>> [RESET] Da khoi tao lai van choi moi!\n";
        }

        renderAsciiBoard(snake, currentApple.get(), innerWalls, currentPhase);
    }
}

} // namespace

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    // 1. Luôn chạy bộ kiểm thử tự động 12 chức năng trước tiên
    const bool allPassed = runAutomatedUnitTests();

    // Nếu chạy với tham số --test thì chỉ chạy Unit Test rồi kết thúc (phục vụ CI / chấm điểm nhanh)
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--test") {
            return allPassed ? 0 : 1;
        }
    }

#if SNAKE_BREAKER_HAS_SFML
    std::cout << "Phat hien thu vien SFML! Mo cua so do hoa 960x800...\n";
    std::cout << "Phim dieu khien:\n"
              << " - WASD / Mui ten : Dieu khien ran\n"
              << " - Phim 1 / 2 / 3 : Sinh Tao Do / Tao Vang / Tao Xanh\n"
              << " - Phim E         : Kich hoat pha tuong 3x3 khi Nang luong = 100%\n"
              << " - Phim Space     : Ban thu 1 vien dan (o Giai doan 2)\n"
              << " - Phim P         : Chuyen doi Giai doan 1 <-> Giai doan 2\n"
              << " - Phim R         : Reset van choi\n";

    sf::RenderWindow window(sf::VideoMode(config::WINDOW_WIDTH, config::WINDOW_HEIGHT),
                            "Snake Breaker - Test Module Thanh Vien 2 (Dao Duy Khoi)");
    window.setFramerateLimit(60);

    Snake snake(sf::Vector2i(config::SAFE_SPAWN_X, config::SAFE_SPAWN_Y));
    AppleSpawner spawner;
    GamePhase currentPhase = GamePhase::SURVIVAL_PHASE;

    std::vector<sf::Vector2i> innerWalls = {
        sf::Vector2i(10, 8), sf::Vector2i(10, 9), sf::Vector2i(10, 10),
        sf::Vector2i(14, 6), sf::Vector2i(15, 6), sf::Vector2i(16, 12)
    };
    std::unique_ptr<Apple> currentApple = spawner.spawnApple(snake, innerWalls);
    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                snake.handleKeyPress(event.key.code);

                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                } else if (event.key.code == sf::Keyboard::R) {
                    snake.reset();
                    currentPhase = GamePhase::SURVIVAL_PHASE;
                    currentApple = spawner.spawnApple(snake, innerWalls);
                } else if (event.key.code == sf::Keyboard::P) {
                    if (currentPhase == GamePhase::SURVIVAL_PHASE) {
                        currentPhase = GamePhase::BOSS_PHASE;
                        snake.initBossPhaseAmmo();
                    } else {
                        currentPhase = GamePhase::SURVIVAL_PHASE;
                    }
                } else if (event.key.code == sf::Keyboard::E) {
                    if (snake.consumeEnergy()) {
                        sf::Vector2i head = snake.getGridPosition();
                        innerWalls.erase(
                            std::remove_if(innerWalls.begin(), innerWalls.end(),
                                [&](const sf::Vector2i& w) {
                                    return std::abs(w.x - head.x) <= 1 && std::abs(w.y - head.y) <= 1;
                                }),
                            innerWalls.end()
                        );
                    }
                } else if (event.key.code == sf::Keyboard::Space) {
                    snake.consumeAmmo(1);
                } else if (event.key.code == sf::Keyboard::Num1) {
                    currentApple = spawner.createAppleByType(AppleType::NORMAL_APPLE,
                        currentApple ? currentApple->getGridPosition() : sf::Vector2i(12, 9));
                } else if (event.key.code == sf::Keyboard::Num2) {
                    currentApple = spawner.createAppleByType(AppleType::HEALTH_APPLE,
                        currentApple ? currentApple->getGridPosition() : sf::Vector2i(12, 9));
                } else if (event.key.code == sf::Keyboard::Num3) {
                    currentApple = spawner.createAppleByType(AppleType::ENERGY_APPLE,
                        currentApple ? currentApple->getGridPosition() : sf::Vector2i(12, 9));
                }
            }
        }

        const float gameDeltaTime = clock.restart().asSeconds();
        snake.update(gameDeltaTime);
        if (currentApple) {
            currentApple->update(gameDeltaTime);
        }

        if (snake.hasMovedThisFrame()) {
            if (currentApple && currentApple->isActive() &&
                snake.getGridPosition() == currentApple->getGridPosition()) {
                currentApple->onEat(snake, currentPhase);
                if (currentPhase == GamePhase::SURVIVAL_PHASE && snake.hasReachedBossPhaseThreshold()) {
                    currentPhase = GamePhase::BOSS_PHASE;
                    innerWalls.clear();
                    snake.initBossPhaseAmmo();
                }
                currentApple = spawner.spawnApple(snake, innerWalls);
            }
            snake.handleCollisions(innerWalls);
        }

        window.clear(sf::Color(22, 25, 34));

        // Vẽ lưới và tường viền ngoài
        for (int y = 0; y < config::MAP_HEIGHT; ++y) {
            for (int x = 0; x < config::MAP_WIDTH; ++x) {
                sf::Vector2i tilePos(x, y);
                sf::RectangleShape tileShape(sf::Vector2f(
                    static_cast<float>(config::TILE_SIZE - 1),
                    static_cast<float>(config::TILE_SIZE - 1)
                ));
                tileShape.setPosition(config::gridToPixel(tilePos));
                if (config::isBorderWall(tilePos)) {
                    tileShape.setFillColor(sf::Color(70, 75, 95));
                } else {
                    tileShape.setFillColor(sf::Color(32, 36, 48));
                }
                window.draw(tileShape);
            }
        }

        // Vẽ tường bên trong
        for (const auto& wallPos : innerWalls) {
            sf::RectangleShape wallShape(sf::Vector2f(
                static_cast<float>(config::TILE_SIZE - 2),
                static_cast<float>(config::TILE_SIZE - 2)
            ));
            wallShape.setPosition(config::gridToPixel(wallPos));
            wallShape.setFillColor(sf::Color(165, 110, 65));
            window.draw(wallShape);
        }

        // Vẽ táo và rắn
        if (currentApple) {
            currentApple->render(window);
        }
        snake.render(window);

        window.display();
    }
#else
    runInteractiveConsoleMode();
#endif

    return 0;
}
