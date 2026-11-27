# Snake Breaker – Core V2 (SFML 3.0.2)

Bản thử nghiệm độc lập của thành viên 1. Game Core và Map là code chính; Snake/Apple ở Game chỉ là demo để kiểm thử, sẽ thay bằng module của thành viên 2 khi tích hợp.

## Cập nhật
- Rắn xanh thân tròn nối liền, có mắt nhìn theo hướng di chuyển. Táo đỏ dùng PNG trong `assets/textures/apple.png` (có hình dự phòng nếu thiếu file). Bản đồ nền caro xanh.
- WASD / mũi tên: bắt đầu di chuyển và đổi hướng. Rắn tự đi mỗi 0,16 giây; đâm tường/thân thì dừng, đổi hướng hợp lệ để đi tiếp. Không có hệ thống mất mạng trong demo.
- Ăn táo tăng chiều dài tối đa 12, cập nhật số táo, sinh táo mới ở ô trống (tránh ô đang cảnh báo). N vẫn là phím giả lập số táo, không tăng chiều dài.
- Tường sinh theo cặp ngang/dọc; 2, 1,5, 1, 0,7 giây theo số táo (0–2, 3–5, 6–8, 9); cảnh báo 0,4 giây. Giới hạn 48 ô tường trong; 80 tường viền không phá được. Hủy cả cặp nếu một ô mất tính hợp lệ. Phá tường vẫn phá từng ô để tương thích module Combat. Khi còn 1 ô sức chứa thì không sinh thêm cặp.
- Đủ 10 táo: dừng sinh tường, xóa tường trong, chuyển BOSS_INCOMING rồi BOSS_FIGHT. Chưa có Boss thật, UI, âm thanh hay Combat.

## Cài bản cập nhật
Sao lưu dự án trước. Nếu đã có project, giải nén và copy `Core/`, `tests/`, `assets/`, `CMakeLists.txt` vào đúng thư mục gốc chứa `main.cpp`, chọn ghi đè file tương ứng; giữ các file của nhóm khác. Không xóa thư mục build cũ nếu vẫn dùng đúng đường dẫn và compiler. Có thể mở thư mục mới của bản V2 thay thế khi chưa tích hợp nhóm.

Mở PowerShell trong thư mục chứa CMakeLists.txt:

```powershell
$env:Path += ";C:\Program Files\CMake\bin"
cmake -S . -B build-mingw -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build-mingw -j 8
.\build-mingw\SnakeBreaker.exe
```

CMake dùng FetchContent và giữ SFML 3.0.2, không cần liên kết gói GCC 14.2.0 thủ công. Nếu thiếu mạng khi cấu hình lần đầu, cần SFML source/cache tương thích. CMake cũng copy assets vào thư mục executable. Nếu đã cấu hình trước, vẫn chạy lại configure khi thay CMakeLists.txt.

## Kiểm thử
- Chưa bấm điều khiển: quan sát tường tự xuất hiện theo cặp.
- WASD/mũi tên, ăn táo, đổi hướng, va tường rồi đổi hướng.
- Chuột trái: xóa 1 ô tường trong (không phải blast).
- N: giả lập ăn 1 táo để thử tăng độ khó và chuyển Boss.
- P: pause/resume, R: reset, Esc: thoát.

```powershell
ctest --test-dir build-mingw --output-on-failure
```

Logic Map đã chạy qua CoreTests với lớp giả lập tối thiểu cho kiểu dữ liệu/đồ họa SFML trong môi trường kiểm tra. Chưa build với SFML thật, chưa chạy cửa sổ Windows; cần xác nhận bằng lệnh trên.

## API tích hợp
Giữ các API public của Game/Map/Wall. `Map::getPendingWall()` vẫn trả ô đầu của cặp, `getPendingWallPair()` mới trả cả cặp (hoặc nullopt). `destroyWall()` vẫn phá từng ô. `m_debug*` và các hàm debug trong Game là tạm thời. Không đưa logic demo này vào module Snake/Apple chính thức.
