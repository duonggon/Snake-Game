# Snake Breaker - Module Thành Viên 2 (Snake & Apple System)
**Thực hiện:** Đào Duy Khôi (MSSV: 24137037)
**Ngôn ngữ:** C++17 | **Thư viện:** SFML

## Cấu trúc thư mục
```text
SnakeBreaker/
├── main.cpp                      # Chương trình kiểm thử độc lập (Hỗ trợ cả cửa sổ SFML & Console Test)
├── CMakeLists.txt                # Cấu hình build CMake chuẩn cho dự án
├── Core/
│   ├── Config.h                  # Toàn bộ hằng số cân bằng game & quy ước chung
│   ├── SfmlCompat.h              # Tự động nhận diện SFML (dùng SFML thật hoặc giả lập khi chưa cài SFML)
│   ├── GameState.h               # Enum trạng thái game và giai đoạn chơi
│   └── Entity.h                  # Lớp cơ sở trừu tượng cho mọi thực thể (Snake, Apple, Wall, Boss)
└── Player/
    ├── Snake.h                   # Lớp Rắn (Điều khiển, di chuyển, mạng, bất tử, năng lượng, đạn)
    ├── Snake.cpp
    ├── Apple.h                   # Lớp cơ sở trừu tượng cho các loại Táo
    ├── Apple.cpp
    ├── NormalApple.h             # Táo đỏ (85%): +1 đốt, +20% năng lượng (GĐ1) / +1 đạn (GĐ2)
    ├── NormalApple.cpp
    ├── HealthApple.h             # Táo vàng (5%): +1 mạng (tối đa 3 mạng)
    ├── HealthApple.cpp
    ├── EnergyApple.h             # Táo xanh dương (10%): +60% năng lượng (GĐ1) / +3 đạn (GĐ2)
    ├── EnergyApple.cpp
    ├── AppleSpawner.h            # Bộ sinh táo ngẫu nhiên theo tỷ lệ tại vị trí hợp lệ
    └── AppleSpawner.cpp
```

## Lưu ý chung (Tuân thủ ReadMe & Quy tắc đặt tên)
- Đặt tên bằng tiếng Anh, chú thích code bằng tiếng Việt.
- Class: `PascalCase` (`Snake`, `NormalApple`, `AppleSpawner`).
- Hàm và biến: `camelCase` (`takeDamage()`, `appleCount`, `gridPosition`).
- Thuộc tính trong class: tiền tố `m_` (`m_lives`, `m_energy`, `m_ammo`, `m_gridPosition`).
- Hằng số: `UPPER_SNAKE_CASE` đặt trong `Core/Config.h` (`TILE_SIZE`, `MAX_AMMO`).
- Biến bool: bắt đầu bằng `is`, `has`, `can` (`m_isAlive`, `m_isInvincible`, `hasAmmo()`, `canBlast()`).
