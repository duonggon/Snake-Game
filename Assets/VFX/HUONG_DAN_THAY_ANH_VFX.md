# Nguồn Tải Hiệu ứng Phá Tường & Hướng dẫn Thay Ảnh (`Assets/VFX`)

Hệ thống `Effects/EffectManager` đã hỗ trợ **cả ảnh dải ngang 1 hàng (`COLS x 1`) lẫn ảnh bảng nhiều hàng (`COLS x ROWS`)** để bạn tải bất kỳ bộ Sprite Sheet miễn phí nào trên mạng về bỏ vào là chạy được ngay!

---

## 1. Top 4 Gói Hiệu ứng Phá Tường / Gạch Đá Vỡ 2D Miễn phí Đẹp Nhất

1. **Pixel Art Explosion Sprite Sheet (Free, CC0) — Skulitom** *(Khuyên dùng số 1 cho gạch vỡ)*  
   - Link tải trực tiếp: https://skulitom.itch.io/pixel-art-explosion-sprite-sheet  
   - **Điểm mạnh**: Miễn phí 100% (CC0), ngoài vụ nổ lửa còn có riêng dải **`shatter` (12 khung hình mảnh vỡ khối đá/gạch + bụi khói)** cực kỳ hợp khi rắn nhấn `Phím F` phá tường!

2. **Pixel Burst VFX: Explosions & Destruction — dotplays** *(Có bản Free Sampler)*  
   - Link tải trực tiếp: https://dotplays.itch.io/pixel-burst-explosions  
   - **Điểm mạnh**: Thiết kế chuyên biệt cho hiệu ứng **sập tường gạch/đá, vỡ thùng gỗ và bụi mảnh vỡ 2D**.

3. **Free Effect and Bullet 16x16 — BDragon1727** *(Rất hợp đồng bộ cả Nổ tường + Đạn bắn Boss)*  
   - Link tải trực tiếp: https://bdragon1727.itch.io/free-effect-and-bullet-16x16  
   - **Điểm mạnh**: Có hàng chục dải Sprite Sheet 16x16 / 32x32 gồm cả chấn động phá tường, khói nổ và viên đạn phát sáng cho Màn 2.

4. **Five Free Pixel Explosions — unTied Games** *(Rất hợp cho vụ nổ tiêu diệt Boss 2x2)*  
   - Link tải trực tiếp: https://untiedgames.itch.io/five-free-pixel-explosions  
   - **Điểm mạnh**: Hoạt ảnh nổ lớn nhiều khung hình cực mượt, có sẵn file PNG Sprite Sheet nền trong suốt.

*(Ngoài ra nếu muốn tự vẽ lại màu gạch cho tiệp 100% với màu ô tường trong game, bạn mở https://www.piskelapp.com/ ngay trên trình duyệt, vẽ 4–6 khung hình gạch nứt rồi bấm **Export -> PNG -> Sprite sheet**).*

---

## 2. Cách Gắn Vào Game Trong 30 Giây (Không Cần Sửa Code C++)

1. Giải nén gói tải về, chọn 1 file `.png` hiệu ứng vỡ tường bạn thích nhất, đổi tên thành **`wall_break.png`** và chép vào thư mục:  
   `e:\OOP\Assets\VFX\wall_break.png`
2. Mở tệp `e:\OOP\Effects\VfxStyleConfig.h` (dòng 30–32), điền đúng **số cột (`COLS`)** và **số hàng (`ROWS`)** khung hình có trong tấm ảnh `.png` đó:
   ```cpp
   inline constexpr std::string_view WALL_BREAK_SPRITESHEET = "Assets/VFX/wall_break.png";
   inline constexpr int WALL_BREAK_SPRITE_COLS = 6; // Ví dụ ảnh có 6 hình xếp ngang
   inline constexpr int WALL_BREAK_SPRITE_ROWS = 1; // Ví dụ 1 hàng (nếu bảng 4x3 thì điền COLS=4, ROWS=3)
   ```
3. Nhấn **`Ctrl+Shift+B`** (hoặc **`F5`**) trong VS Code — mỗi khi bấm `Phím F`, game sẽ tự động phát hoạt ảnh từ file `wall_break.png` của bạn trong `0.5 giây`!
