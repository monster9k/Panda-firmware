# PANDA – Robot đồng hành thông minh hỗ trợ học tiếng Nhật

Robot để bàn dùng giọng nói làm kênh tương tác chính, giúp sinh viên luyện nghe – nói từ vựng và mẫu câu tiếng Nhật. Robot phát âm mẫu (TTS), nhận diện và chấm câu trả lời của người học (STT + fuzzy matching), phản hồi bằng chuyển động, biểu cảm khuôn mặt và chữ Hán/Kana trên màn hình OLED. Dữ liệu tương tác truyền qua MQTT trên nền tảng IoT và lưu trên server để sinh viên theo dõi tiến độ học tập qua giao diện web.

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
