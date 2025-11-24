\# THE GAME OF GO (CỜ VÂY) - FINAL PROJECT



!\[Project Banner](resources/images/gameplay\_background.png) 

\*(Hình ảnh minh họa: Giao diện bàn cờ)\*



\## 📖 Giới thiệu



\*\*The Game of Go\*\* là dự án môn \*\*Introduction Computer Science\*\*, được phát triển hoàn toàn bằng ngôn ngữ \*\*C++\*\* kết hợp với thư viện đồ họa đa phương tiện \*\*SFML\*\*.



---



\## ✨ Tính năng Nổi bật



\### 🎮 Chế độ chơi (Gameplay)

\- \*\*PvP (Người đấu Người):\*\* Hai người chơi thay phiên nhau trên cùng một máy. Hệ thống tự động quản lý lượt đi.

\- \*\*PvAI (Người đấu Máy):\*\* Chế độ luyện tập với máy (Random Strategy).

\- \*\*Luật Cờ Chuẩn Quốc Tế:\*\*

&nbsp; - \*\*Bắt quân (Capture):\*\* Tự động phát hiện và loại bỏ quân hết khí.

&nbsp; - \*\*Luật Ko:\*\* Ngăn chặn vòng lặp vô tận.

&nbsp; - \*\*Luật Tự tử (Suicide):\*\* Cấm nước đi vào ô không có khí (trừ khi ăn quân).

\- \*\*Time Limit:\*\* Hệ thống đồng hồ đếm ngược (Countdown Timer). Tự động xử thua khi hết giờ.



\### 🎨 Giao diện \& Hiệu ứng (Visuals)

\- \*\*Scoring Overlay (Độc đáo):\*\* Sử dụng thuật toán loang (Flood Fill) để hiển thị lãnh thổ bằng hiệu ứng sóng màu lan tỏa (Wave Animation) cực kỳ trực quan.

\- \*\*Dynamic Timeline:\*\* Thanh lịch sử phía trên màn hình hiển thị thời gian suy nghĩ của từng nước đi thông qua độ rộng các ô màu.

\- \*\*Interactive History:\*\* Danh sách nước đi bên phải tự động cuộn theo ván đấu.



\### 🛠️ Hệ thống \& Tiện ích

\- \*\*Save/Load Không Giới Hạn:\*\*

&nbsp; - Lưu trữ bao nhiêu ván cờ tùy thích.

&nbsp; - \*\*Thumbnail Generation:\*\* Tự động chụp ảnh bàn cờ hiện tại và lưu thành file ảnh `.png` để xem trước khi Load.

\- \*\*Undo/Redo:\*\* Quay lui và làm lại nước đi không giới hạn số lượng (sử dụng cấu trúc Stack).

\- \*\*Global Settings:\*\* Tùy chỉnh Âm lượng, Kích thước bàn cờ (9x9, 13x13, 19x19), Điểm chấp (Komi) và lưu cấu hình vĩnh viễn.



---



\## 🚀 Hướng dẫn Cài đặt \& Chạy Game



\### Cách 1: Chơi ngay (Portable Version) - Khuyên dùng

Trong gói nộp bài, tôi đã chuẩn bị sẵn bản chạy ngay không cần cài đặt.



1\.  Tìm file \*\*`GoGame.exe`\*\* nằm ngay tại thư mục gốc.

2\.  Click đúp chuột để chạy.

3\.  Thưởng thức!



\*(Lưu ý: Đảm bảo thư mục `resources` và các file `.dll` nằm cùng cấp với file .exe)\*



\### Cách 2: Biên dịch từ Mã nguồn (Build from Source)

Nếu bạn muốn kiểm tra code hoặc build lại dự án:



\*\*Yêu cầu hệ thống:\*\*

\- \*\*IDE:\*\* Code::Blocks 20.03 (MinGW version).

\- \*\*Compiler:\*\* GCC MinGW-w64 13.1.0 (64-bit).

\- \*\*Library:\*\* SFML 2.6.1 (GCC MinGW 64-bit).



\*\*Các bước thực hiện:\*\*

1\.  Mở file `GoGame.cbp` bằng Code::Blocks.

2\.  Vào menu \*\*Project -> Build options\*\*. Đảm bảo đường dẫn `Search directories` trỏ đúng đến thư mục SFML trên máy bạn.

3\.  Nhấn \*\*F9\*\* (Build and Run).



---



\## 🎮 Hướng dẫn Chơi



\### Điều khiển Cơ bản

\- \*\*Chuột trái:\*\*

&nbsp; - Đặt quân cờ lên bàn.

&nbsp; - Tương tác với các nút bấm (Pause, Undo, Pass...).

&nbsp; - Kéo thanh trượt (Volume, History Scrollbar).

\- \*\*Lăn chuột:\*\* Cuộn danh sách lịch sử nước đi hoặc nội dung trang About.

\- \*\*ESC:\*\* Mở nhanh Menu Tạm dừng (Pause).



\### Quy trình một ván cờ

1\.  Chọn \*\*New Game\*\* -> Chọn chế độ (PvP/PvAI).

2\.  Chọn Kích thước bàn cờ và Độ khó (nếu chơi với máy).

3\.  Hai bên lần lượt đi quân. Mục tiêu là vây chiếm lãnh thổ.

4\.  Nếu thấy không còn nước đi hiệu quả, bấm \*\*PASS\*\*.

5\.  Khi cả 2 bên cùng PASS, game kết thúc.

6\.  Hệ thống sẽ chạy hiệu ứng tính điểm và hiển thị người thắng cuộc dựa trên: `Đất + Quân đã ăn + Komi`.



---



\## 📂 Cấu trúc Thư mục



```text

GoGame\_Project/

├── GoGame.exe           # File thực thi chính

├── src/                 # Mã nguồn (.cpp) - Logic game, AI, UI

├── include/             # File tiêu đề (.h) - Khai báo lớp

├── resources/           # Tài nguyên game (BẮT BUỘC)

│   ├── images/          # Hình ảnh bàn cờ, quân cờ, nút bấm

│   ├── sounds/          # Âm thanh đặt quân, nhạc nền

│   └── fonts/           # Font chữ

└── \*.dll                # Các thư viện động (SFML, MinGW)



