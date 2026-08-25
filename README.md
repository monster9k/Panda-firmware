# PANDA – Robot đồng hành thông minh hỗ trợ trẻ em học tiếng Anh

Robot để bàn dùng giọng nói làm kênh tương tác chính, giúp trẻ em làm quen và luyện nghe – nói từ vựng tiếng Anh. Robot phát âm mẫu (TTS), nhận diện và chấm câu trả lời của trẻ (STT + fuzzy matching), phản hồi bằng chuyển động, biểu cảm khuôn mặt và từ vựng/hình ảnh minh họa tiếng Anh trên màn hình OLED. Robot còn nhận lệnh di chuyển bằng giọng nói tiếng Anh/Việt cơ bản ("Go", "Stop", "tiến lên", "dừng lại") để tương tác vận động cùng trẻ, và né vật cản bằng cảm biến siêu âm. Dữ liệu tương tác truyền qua MQTT trên nền tảng IoT và lưu trên server để phụ huynh/sinh viên theo dõi tiến độ học tập qua giao diện web.

> Lưu ý phạm vi: đề xuất ban đầu của nhóm hướng tới học tiếng Nhật hai chiều (Việt↔Nhật). Nhóm đã thu hẹp phạm vi thành dạy tiếng Anh một chiều cho trẻ em — lý do và phân tích kỹ thuật xem mục "Quyết định thu hẹp phạm vi" trong [PROJECT.md](PROJECT.md).

## Thành viên nhóm

| Họ và tên | Nhiệm vụ | Vai trò |
|---|---|---|
| Nguyễn Ngọc Anh | Vi điều khiển & Web | Nhóm trưởng |
| Hoàng Đình Chiến Thắng | Vi điều khiển & Báo cáo | Thành viên |
| Nguyễn Hữu Thái | AI & Web | Thành viên |
| Nguyễn Viết Minh Khoa | Firmware nhúng & AI | Thành viên |

## Kiến trúc tổng thể

```
ESP32 EDGE NODE (Camera + Mic + Speaker + OLED + Motor)
        | Wi-Fi + MQTT/WebSocket
        v
CLOUD SERVER (AI Backend)
  - Database (User Profile, Vocabulary History, Progress Tracking)
  - AI Pipeline: STT -> NLU -> Grammar Checker -> Conversation Engine -> Quiz Generator -> TTS
  - Computer Vision: Face Recognition, Emotion Detection, Engagement Tracking
```

Chi tiết đầy đủ xem trong tài liệu đề xuất PBL4 của nhóm.

## Cấu trúc repo

```
PANDA/
├── README.md            # File này
├── PROJECT.md            # Nhật ký/kế hoạch cá nhân của Khoa (firmware)
├── CLAUDE.md              # Hướng dẫn cho Claude Code khi làm việc trong repo
├── .claude/               # Cấu hình Claude Code (permissions...)
└── firmware-demo/         # Demo firmware mô phỏng (Wokwi) — ESP32 + OLED + nút bấm + MQTT
    ├── platformio.ini
    ├── wokwi.toml
    ├── diagram.json
    ├── src/main.cpp
    └── web-dashboard/index.html
```

## Chạy thử demo firmware (không cần phần cứng thật)

1. Cài VS Code + extension **PlatformIO IDE** + **Wokwi for VS Code**.
2. Mở thư mục `firmware-demo/` trong VS Code.
3. Build: `pio run` (kiểm tra code compile sạch).
4. Nhấn `F1` → `Wokwi: Start Simulator` (hoặc mở `wokwi.com`, kéo thả các file trong `firmware-demo/` vào) để chạy mô phỏng ESP32 + OLED + 2 nút bấm.
5. Mở `firmware-demo/web-dashboard/index.html` bằng trình duyệt để xem bảng tiến độ realtime nhận qua MQTT.

Chi tiết kế hoạch từng bước xem [PROJECT.md](PROJECT.md).
