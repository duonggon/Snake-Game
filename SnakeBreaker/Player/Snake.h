#pragma once
#include <vector>
#include "../Core/Entity.h"
#include "../Core/GameState.h"

// ============================================================================
// File: Player/Snake.h
// Phụ trách: Thành viên 2 – Đào Duy Khôi (Snake & Apple System)
// Mô tả: Lớp Snake kế thừa từ Entity, quản lý toàn bộ cơ chế của người chơi:
//        - Điều khiển bằng WASD và phím mũi tên (chống quay đầu 180 độ).
//        - Tự động di chuyển theo lưới với Delta Time.
//        - Tăng chiều dài khi ăn táo (khởi đầu 3 đốt, tối đa 12 đốt).
//        - Kiểm tra va chạm với thân rắn, tường viền ngoài, tường bên trong.
//        - Quản lý 3 mạng, hồi sinh tại vị trí an toàn khi mất mạng.
//        - Bất tử 1,5 giây sau khi mất mạng kèm hiệu ứng nhấp nháy.
//        - Quản lý năng lượng phá tường (0% - 100%) và số đạn (tối đa 5 viên).
// ============================================================================

class Snake : public Entity {
public:
    // Khởi tạo rắn tại vị trí an toàn mặc định
    Snake();
    explicit Snake(const sf::Vector2i& startGridPosition);
    ~Snake() override = default;

    // Đặt lại toàn bộ trạng thái rắn về ban đầu (dùng khi bắt đầu ván mới / Restart)
    void reset(const sf::Vector2i& startGridPosition = sf::Vector2i(config::SAFE_SPAWN_X, config::SAFE_SPAWN_Y));

    // ------------------------------------------------------------------------
    // 1. ĐIỀU KHIỂN & DI CHUYỂN (WASD / Phím mũi tên)
    // ------------------------------------------------------------------------
    // Xử lý phím bấm từ người chơi (WASD hoặc Up/Down/Left/Right)
    void handleKeyPress(sf::Keyboard::Key key);

    // Đổi hướng di chuyển (tự động chặn quay đầu ngược 180 độ trực tiếp)
    void setDirection(Direction newDirection);

    // Cập nhật trạng thái rắn theo thời gian mô phỏng game (gameDeltaTime)
    void update(float gameDeltaTime) override;

    // Vẽ rắn lên cửa sổ SFML (có hiệu ứng nhấp nháy khi đang bất tử)
    void render(sf::RenderWindow& window) override;

    // Thực hiện 1 bước di chuyển trên lưới (có thể gọi trực tiếp khi kiểm thử)
    void moveStep();

    // ------------------------------------------------------------------------
    // 2. TĂNG CHIỀU DÀI & KIỂM TRA VỊ TRÍ TRÊN LƯỚI
    // ------------------------------------------------------------------------
    // Tăng chiều dài rắn thêm segments đốt (giới hạn tối đa MAX_SNAKE_LENGTH = 12)
    void grow(int segments = config::NORMAL_APPLE_GROWTH);

    // Kiểm tra một ô lưới có trùng với bất kỳ đốt nào của rắn (đầu hoặc thân) không
    bool isOccupying(const sf::Vector2i& gridPosition) const;

    // Kiểm tra một ô lưới có trùng với phần thân rắn (không tính đầu) không
    bool isBodyOccupying(const sf::Vector2i& gridPosition) const;

    // Tính khoảng cách Chebyshev (max(|dx|, |dy|)) từ một ô tới đầu rắn
    // (Hỗ trợ Thành viên 1 không sinh tường trong phạm vi 2 ô quanh đầu rắn)
    int getDistanceToHead(const sf::Vector2i& gridPosition) const;

    // Tính khoảng cách ngắn nhất từ một ô tới bất kỳ đốt nào của thân rắn
    // (Hỗ trợ Thành viên 3 kiểm tra Boss dịch chuyển cách thân rắn ít nhất 3 ô)
    int getMinDistanceToBody(const sf::Vector2i& gridPosition) const;

    // ------------------------------------------------------------------------
    // 3. XỬ LÝ VA CHẠM, QUẢN LÝ MẠNG, HỒI SINH & BẤT TỬ
    // ------------------------------------------------------------------------
    // Kiểm tra đầu rắn có tự va chạm với thân rắn hay không
    bool checkSelfCollision() const;

    // Kiểm tra đầu rắn có va chạm với tường viền ngoài hoặc tường bên trong không
    bool checkWallCollision(const std::vector<sf::Vector2i>& innerWalls) const;

    // Kiểm tra và xử lý tổng hợp va chạm (tường + thân rắn + Boss nếu có)
    // Trả về true nếu rắn bị mất mạng trong lần kiểm tra này
    bool handleCollisions(const std::vector<sf::Vector2i>& innerWalls,
                          const std::vector<sf::Vector2i>& bossTiles = {});

    // Nhận sát thương (mất damageAmount mạng, đưa về vị trí an toàn và bật bất tử 1,5s)
    // Trả về true nếu rắn thực sự bị trừ mạng (không bị trừ nếu đang bất tử)
    bool takeDamage(int damageAmount = 1,
                    const std::vector<sf::Vector2i>& innerWalls = {},
                    const std::vector<sf::Vector2i>& bossTiles = {});

    // Hồi mạng khi ăn táo vàng (HealthApple) - không vượt quá MAX_LIVES (3 mạng)
    void addLife(int amount = config::HEALTH_APPLE_LIFE);

    // Đưa rắn về vị trí an toàn không trùng tường/Boss, giữ nguyên chiều dài hiện tại
    void respawnAtSafePosition(const std::vector<sf::Vector2i>& innerWalls = {},
                               const std::vector<sf::Vector2i>& bossTiles = {});

    // Kích hoạt trạng thái bất tử trong khoảng thời gian duration (mặc định 1,5 giây)
    void activateInvincibility(float duration = config::INVINCIBLE_DURATION);

    // ------------------------------------------------------------------------
    // 4. QUẢN LÝ NĂNG LƯỢNG PHÁ TƯỜNG (Giai đoạn 1)
    // ------------------------------------------------------------------------
    // Cộng năng lượng phá tường (giới hạn tối đa MAX_ENERGY = 100%)
    void addEnergy(int amount);

    // Kiểm tra năng lượng đã đạt 100% để kích hoạt kỹ năng phá tường (phím E) chưa
    bool canBlast() const;

    // Tiêu hao toàn bộ 100% năng lượng khi kích hoạt phá tường (đưa về 0%)
    // Phương thức giao tiếp để CombatSystem (Thành viên 4) gọi
    bool consumeEnergy();

    // ------------------------------------------------------------------------
    // 5. QUẢN LÝ SỐ ĐẠN (Giai đoạn 2 - Đánh Boss)
    // ------------------------------------------------------------------------
    // Cấp 3 viên đạn ban đầu khi chuyển sang giai đoạn đánh Boss (ăn đủ 10 táo)
    void initBossPhaseAmmo();

    // Cộng thêm đạn khi ăn táo ở giai đoạn 2 (giới hạn tối đa MAX_AMMO = 5 viên)
    void addAmmo(int amount);

    // Kiểm tra rắn còn đạn hay không
    bool hasAmmo() const;

    // Kiểm tra rắn có thể bắn súng hay không (còn sống và còn ít nhất 1 viên đạn)
    bool canShoot() const;

    // Tiêu hao đạn mỗi khi bắn (kể cả bắn trượt)
    // Phương thức giao tiếp để CombatSystem (Thành viên 4) gọi
    bool consumeAmmo(int amount = 1);

    // ------------------------------------------------------------------------
    // 6. QUẢN LÝ SỐ TÁO ĐÃ ĂN & GETTERS CHO HUD / GAME CORE
    // ------------------------------------------------------------------------
    void incrementAppleCount();
    bool hasReachedBossPhaseThreshold() const;

    int getLives() const { return m_lives; }
    int getEnergy() const { return m_energy; }
    int getAmmo() const { return m_ammo; }
    int getAppleCount() const { return m_appleCount; }
    int getLength() const { return static_cast<int>(m_body.size()); }
    float getSpeed() const { return m_speed; }
    void setSpeed(float speed);

    Direction getDirection() const { return m_direction; }
    Direction getNextDirection() const { return m_nextDirection; }

    bool isAlive() const { return m_isAlive; }
    bool isInvincible() const { return m_isInvincible; }
    bool isVisible() const { return m_isVisible; }
    float getInvincibleTimer() const { return m_invincibleTimer; }
    bool hasMovedThisFrame() const { return m_hasMovedThisFrame; }

    const std::vector<sf::Vector2i>& getBody() const { return m_body; }

private:
    // Kiểm tra hai hướng có ngược 180 độ với nhau hay không
    static bool isOppositeDirection(Direction currentDir, Direction newDir);

    // Tính tọa độ ô tiếp theo dựa trên hướng di chuyển
    static sf::Vector2i getDirectionOffset(Direction dir);

private:
    std::vector<sf::Vector2i> m_body; // Các đốt của rắn (m_body[0] là đầu rắn)
    Direction m_direction;            // Hướng di chuyển thực tế của bước gần nhất
    Direction m_nextDirection;        // Hướng di chuyển dự kiến cho bước tiếp theo

    float m_speed;                    // Tốc độ di chuyển (số ô / giây)
    float m_moveTimer;                // Bộ đếm thời gian tích lũy cho bước di chuyển
    int m_pendingGrowth;              // Số đốt đang chờ tăng thêm khi rắn di chuyển

    int m_lives;                      // Số mạng hiện tại của rắn (tối đa 3)
    int m_energy;                     // Năng lượng phá tường (0 - 100%)
    int m_ammo;                       // Số đạn hiện tại (0 - 5 viên)
    int m_appleCount;                 // Tổng số táo đã ăn (mọi loại táo)

    bool m_isAlive;                   // Trạng thái còn sống của rắn (m_lives > 0)
    bool m_isInvincible;              // Trạng thái bất tử sau khi mất mạng
    float m_invincibleTimer;          // Thời gian bất tử còn lại (giây)
    float m_blinkTimer;               // Bộ đếm chu kỳ nhấp nháy khi bất tử
    bool m_isVisible;                 // Trạng thái hiển thị sáng/mờ khi nhấp nháy
    bool m_hasMovedThisFrame;         // Cờ đánh dấu rắn vừa dịch chuyển sang ô mới
};
