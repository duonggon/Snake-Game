# Hướng dẫn Kết nối Module Chiến đấu & VFX cho Cả Nhóm (`TEAM_INTEGRATION_GUIDE.md`)

**Người phụ trách module `Combat/` & `Effects/`:** Huỳnh Nhựt Cường (MSSV: 24137045)  
**Chuẩn C++ & Thư viện:** C++17 — **SFML 3.1.0** (WinLibs MinGW GCC 14.2.0 UCRT 64-bit + CMake)

Để khi ghép code cả 5 thành viên không bị xung đột tên hàm hay lệch cơ chế gameplay, mọi người lưu ý các điểm kết nối dưới đây nhé!

---

## 1. Thống nhất Quy tắc Gameplay Đã Chốt ở Module Chiến đấu

1. **Tách biệt rõ Màn 1 và Màn 2 (`GameStage`)**:
   - **Màn 1 (`GameStage::Stage1_Survival` — Sinh tồn)**: Rắn chỉ di chuyển ăn táo tăng đốt, né tường mọc ngẫu nhiên và dùng **`Phím Space` để phá tường**. **Không mở hệ thống súng** ở Màn 1.
   - **Màn 2 (`GameStage::Stage2_BossFight` — Đánh Boss)**: Giữ kỹ năng phá tường `Phím Space`, đồng thời **mở khóa hệ thống bắn Boss (`Chuột Phải` Toggle ngắm + `Chuột Trái` bắn)**.
2. **Kỹ năng phá tường (`Phím Space` — `0.0s` hồi chiêu) — Dùng đốt rắn thay cho thanh năng lượng riêng**:
   - Dùng **`Phím Space`** (ngón cái tay trái) để nổ phá tường liên tục tức thì (`WALL_BREAK_COOLDOWN = 0.0f`), không đụng chạm các ngón lái `WASD`.
   - Khi rắn ăn táo sẽ dài thêm `1` đốt. Mỗi lần bấm `Space` nổ vùng `3 × 3` quanh đầu rắn sẽ **trừ đi `1` đốt đuôi rắn** (bất kể xung quanh có tường hay không).
   - **Ngưỡng tối thiểu**: Khi rắn chỉ còn **`<= 3` đốt** thì **không thể** bấm `Space` phá tường được nữa (cần từ `4` đốt trở lên).
   - **Va chạm tường**: Nếu đầu rắn đâm trực diện vào tường chưa kịp phá, **rắn `DIE` ngay lập tức**.
3. **Hệ thống ngắm bắn Boss ở Màn 2 (`Chuột Phải` Toggle + `Chuột Trái` Bắn)**:
   - Click **`Chuột Phải`** (**Toggle**, thời gian giữ ngắm tối đa mặc định **`10.0 giây`**) để bật chế độ ngắm + làm chậm thế giới còn **`10%`** (`timeScale = 0.10f`).
   - Click **`Chuột Trái`** bắn `1` viên đạn (`-1` đạn, trúng Boss `2 × 2 ô` gây **`15 HP`** sát thương, đường đạn hiển thị dạng **nét đứt nối từ đầu rắn đến viên đạn**).
   - Nếu click chuột ngoài vòng bán kính `5 ô`, súng vẫn nổ (`-1` đạn) nhưng viên đạn chỉ bay tối đa tới **rìa vòng `5 ô`**.
   - Ngay sau khi bắn `1` viên (hoặc nhấn `Chuột Phải` lần 2 tắt ngắm / hết `10.0s` giữ ngắm), hệ thống **tự động thoát ngắm ngay lập tức** và vào **hồi chiêu `5.0 giây`**.

---

## 2. Những Hàm Cần Có ở Từng Thành viên Để Kết nối (`Combat/CombatInterfaces.h`)

Module của Cường không sửa trực tiếp vào bên trong class của các bạn mà giao tiếp qua 3 interface siêu gọn trong `Combat/CombatInterfaces.h`:

### A. Dương Vĩnh Gôn (*Game Core & Map — 24 × 18 ô*)
1. **Trong `Map` (kế thừa `ICombatMap` hoặc viết bộ chuyển 4 hàm)**:
   - `bool isInsideBounds(int col, int row) const`: Kiểm tra `0 <= col < 24` và `0 <= row < 18`.
   - `bool isDestructibleWall(int col, int row) const`: Trả về `true` **chỉ với tường phá hủy được**, trả về `false` với ô trống và tường cố định (`FixedWall`).
   - `void destroyWall(int col, int row)`: Biến ô tường đó thành ô trống.
   - `sf::Vector2f gridToWorldCenter(int col, int row) const`: Đổi tọa độ ô `(col, row)` sang tọa độ pixel tâm ô trên cửa sổ.
2. **Trong `Game Loop` (`Core/Game.cpp`)**:
   - Tách thời gian thực `unscaledDt` và thời gian thế giới `scaledDt = unscaledDt * combatSystem.getTimeScale();`:
     - Truyền `unscaledDt` cho: `combatSystem.update(...)`, `effectManager.update(unscaledDt)`, `screenShake.update(unscaledDt)`.
     - Truyền `scaledDt` cho: di chuyển của `Snake`, di chuyển của `Boss`, và bộ đếm sinh tường ngẫu nhiên của `Map` (để khi bật Slow Motion mọi thứ chậm đúng `30%`).
   - Khi chuyển từ giai đoạn Sinh tồn sang gặp Boss, gọi:
     ```cpp
     combatSystem.setGameStage(GameStage::Stage2_BossFight, &effectManager);
     ```
   - Khi vẽ khung hình và lấy tọa độ chuột:
     ```cpp
     // Lấy tọa độ chuột bằng baseWorldView tĩnh (trước khi áp dụng rung màn hình)
     sf::Vector2f mouseWorld = window.mapPixelToCoords(sf::Mouse::getPosition(window), baseWorldView);

     // Vẽ bản đồ, rắn, boss, hiệu ứng thế giới bằng View rung nhẹ
     window.setView(screenShake.applyToView(baseWorldView));
     // ... vẽ Map, Snake, Boss ...
     combatSystem.renderAimingWorldUI(window, mouseWorld, map, snake, &boss);
     effectManager.renderWorldEffects(window);

     // Chuyển lại View tĩnh để vẽ lớp phủ màu Slow Motion mượt & HUD
     window.setView(baseWorldView);
     effectManager.renderScreenOverlay(window, {windowWidth, windowHeight});
     ```

---

### B. Đào Duy Khôi (*Player / Rắn & Vật phẩm*)
Cho class `Snake` kế thừa `ICombatPlayer` (hoặc cung cấp đủ các hàm tương ứng):
- `bool isAlive() const`: Trả về trạng thái sống của rắn (nhớ xử lý khi đầu rắn đâm vào tường thì rắn `die` ngay).
- `sf::Vector2i getHeadGridPos() const`: Tọa độ ô `(col, row)` hiện tại của đốt đầu rắn.
- `int getSegmentCount() const`: Tổng số đốt hiện tại của rắn (ăn táo tăng `+1` đốt).
- `void shrinkSegments(int count)`: Cắt bớt `count` đốt từ đuôi rắn (khi bấm `F` phá tường sẽ truyền `count = 1`).
- `int getAmmo() const` và `bool consumeAmmo(int count)`: Lấy và trừ số viên đạn ở Màn 2.
- **Quy tắc sinh Táo & Nhịp mọc tường gợi ý dùng chung (`Core/SpawnRules.h`)**:
  - Đã viết sẵn hàm `SpawnRules::pickNearbySafeItemCell(...)` đảm bảo **táo không bao giờ mọc sát mép/góc bản đồ** (`col, row` cách rìa `>= 2` ô, không kẹt ngõ cụt) và **ưu tiên mọc gần đầu rắn (`2..6` ô)** để không bị bẫy chết ở góc tường.
  - Hàm `SpawnRules::computeWallSpawnInterval(applesEaten)` giúp tường mọc nhanh hơn (`1.15s` cơ bản, giảm dần xuống `0.55s` khi ăn nhiều táo).

---

### C. Lê Đào Tấn Đạt (*Boss quả cầu 2 × 2 ô — 100 HP*)
Cho class `Boss` kế thừa `ICombatBoss`:
- `bool isActive() const`: Trả về `true` khi Boss đã xuất hiện ở Màn 2.
- `bool isAlive() const`: Trả về `hp > 0`.
- `sf::FloatRect getWorldHitbox() const`: Trả về khung hình chữ nhật `2 × 2 ô` của Boss theo tọa độ thế giới (pixel).
- `bool takeDamage(int damageAmount)`: Trừ `damageAmount` (`15 HP`) vào máu của Boss (kèm bật cờ nhấp nháy của Boss nếu có), và **trả về `true` nếu phát bắn này làm máu Boss giảm về `0`** để `CombatSystem` tự động kích hoạt vụ nổ tiêu diệt Boss!

---

### D. Lê Nhật Hoàng (*UI/HUD, Âm thanh & Thống kê*)
1. **Màn hình Hướng dẫn (`Instructions`) & HUD**:
   - Ghi rõ phím phá tường là **`Phím F`** (*Cần rắn > 3 đốt, mỗi lần dùng trừ 1 đốt*).
   - Ở Màn 2 hiển thị thêm trạng thái ngắm bắn: `combatSystem.getCooldownRemaining()` (đếm ngược `5.0s` hồi chiêu) và số đạn hiện có.
2. **Phát Âm thanh không bị phụ thuộc vòng tròn**:
   - Mỗi frame đọc `const CombatEventFlags& ev = combatSystem.getLastFrameEvents();`:
     - `if (ev.wallBreakActivated)` -> phát tiếng nổ phá tường.
     - `if (ev.gunShotFired)` -> phát tiếng bắn súng.
     - `if (ev.bossHit)` -> phát tiếng Boss trúng đạn.
     - `if (ev.bossDefeated)` -> phát tiếng nổ Boss / nhạc chiến thắng.
3. **Màn hình Tổng kết Thống kê cuối game**:
   - Đọc trực tiếp `const CombatStats& stats = combatSystem.getStats();`:
     - `stats.totalWallsDestroyed`: Tổng số tường đã phá.
     - `stats.wallBreakUsedCount`: Số lần sử dụng kỹ năng.
     - `stats.shotsFired`: Tổng số đạn đã bắn.
     - `stats.getAccuracyPercent()`: Tỷ lệ bắn chính xác (`%`).
