#pragma once

// ============================================================================
// File: Core/GameState.h
// Mô tả: Định nghĩa các Enum trạng thái game, giai đoạn màn chơi và hướng di chuyển.
//        Tuân thủ quy tắc: Tên Enum viết giống tên Class (PascalCase),
//        giá trị Enum viết hoa toàn bộ (UPPER_SNAKE_CASE).
// ============================================================================

// Các trạng thái màn hình chính của trò chơi
enum class GameState {
    MAIN_MENU,
    COUNTDOWN,
    PLAYING,
    PAUSED,
    BOSS_INCOMING,
    VICTORY,
    GAME_OVER
};

// Hai giai đoạn chơi trong 1 màn của Snake Breaker
enum class GamePhase {
    SURVIVAL_PHASE, // Giai đoạn 1: Rắn sinh tồn và phá tường (ăn < 10 quả táo)
    BOSS_PHASE      // Giai đoạn 2: Đánh Rolling Boss (sau khi ăn đủ 10 quả táo)
};

// Bốn hướng di chuyển trên lưới 2D
enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};
