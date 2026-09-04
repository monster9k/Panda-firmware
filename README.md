# PANDA – Robot đồng hành thông minh hỗ trợ trẻ em học tiếng Anh

Robot để bàn dùng giọng nói làm kênh tương tác chính, giúp trẻ em làm quen và luyện nghe – nói từ vựng tiếng Anh. Robot phát âm mẫu (TTS), nhận diện và chấm câu trả lời của trẻ (STT + fuzzy matching), phản hồi bằng chuyển động, biểu cảm khuôn mặt và từ vựng/hình ảnh minh họa tiếng Anh trên màn hình OLED. Robot còn nhận lệnh di chuyển bằng giọng nói tiếng Anh/Việt cơ bản ("Go", "Stop", "tiến lên", "dừng lại") để tương tác vận động cùng trẻ, và né vật cản bằng cảm biến siêu âm. Dữ liệu tương tác truyền qua MQTT trên nền tảng IoT và lưu trên server để phụ huynh/sinh viên theo dõi tiến độ học tập qua giao diện web.

> Lưu ý phạm vi: đề xuất ban đầu của nhóm hướng tới học tiếng Nhật hai chiều (Việt↔Nhật). Nhóm đã thu hẹp phạm vi thành dạy tiếng Anh một chiều cho trẻ em — lý do và phân tích kỹ thuật xem mục "Quyết định thu hẹp phạm vi" trong [PROJECT.md](PROJECT.md). **Cập nhật (03/09/2026)**: code AI thật trong `Panda-Robotics-Client-/` cho thấy phạm vi triển khai thực tế đã rộng hơn (hội thoại tiếng Việt tự do, không chỉ chấm từ vựng) — xem [ai.md](ai.md) mục 8, cần nhóm thống nhất lại.

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

## Danh sách linh kiện phần cứng (BOM)

Robot thật dùng **2 board ESP32 riêng biệt** dùng chung 1 khối nguồn: **HEAD** (đầu — camera, mic, loa, 2 mắt OLED) và **BODY** (thân — di chuyển, cảm biến, nút bấm). Tổng chi phí ước tính **~1.425.000đ** (chưa gồm VPS).

### HEAD — ~525k

| # | Linh kiện | SL | Giá ~ | Ghi chú |
|---|---|---|---|---|
| 1 | ESP32-S3 WROOM N16R8 + OV3660 | 1 | 335k | Hộp phải có camera dây dẹt |
| 2 | Mic INMP441 (I2S) | 1 | 30k | — |
| 3 | MAX98357 (amp I2S) | 1 | 40k | — |
| 4 | Loa 8Ω 2–3W | 1 | 30k | Cỡ vừa vỏ |
| 5 | OLED SSD1306 128×64 I2C trắng | 2 | 90k | 4 chân I2C, không mua bản SPI |

### BODY — ~430k

| # | Linh kiện | SL | Giá ~ | Ghi chú |
|---|---|---|---|---|
| 6 | ESP32 DevKit C | 1 | 150k | microUSB |
| 7 | TB6612FNG | 2 | 90k | Thừa 1 chống DOA (hàng lỗi sẵn) |
| 8 | Kit khung 2WD (motor+bánh+xe bò) | 1 | 110k | Đủ ốc |
| 9 | HC-SR04 | 2 | 60k | Thừa 1 |
| 10 | Buzzer active 5V | 1 | 15k | Đúng loại active |
| 11 | Nút nhấn tact | 2 | 5k | — |

### POWER — ~220k

| # | Linh kiện | SL | Giá ~ | Ghi chú |
|---|---|---|---|---|
| 12 | Pin 18650 mới | 2 | 140k | KHÔNG dùng cell laptop cũ |
| 13 | Holder 2 slot | 1 | 30k | Có dây ra |
| 14 | TP4056 có bảo vệ | 1 | 20k | Bản 2 chip |
| 15 | Boost 5V MT3608 | 1 | 20k | — |
| 16 | Công tắc gạt | 1 | 10k | — |

### Phụ kiện + vỏ — ~250k

| # | Linh kiện | SL | Giá ~ |
|---|---|---|---|
| 17 | Breadboard mini + jumper | 1 bộ | 60k |
| 18 | Dây microUSB | 2 | 40k |
| 19 | Vít + trụ đồng + keo nến | 1 bộ | 50k |
| 20 | Vỏ mica / in 3D | 1 | 100k |

### Não cloud (giờ G)

| # | Khoản | Giá ~ |
|---|---|---|
| 21 | VPS 1 tháng (hoặc trial) | 0–200k |

> **Lưu ý cho firmware**: `firmware-demo/` hiện tại (Wokwi) mới mô phỏng 1 board ESP32 + 1 OLED + 2 nút bấm — bản rút gọn để học/demo, **chưa** tách 2 board HEAD/BODY và chưa có mic/amp I2S thật (Wokwi chưa hỗ trợ mô phỏng I2S — xem `WEEKLY_LOGIC.md`, không push git, của Khoa). Khi có phần cứng thật (dự kiến Tuần 9 theo `PROJECT.md`), việc phát triển firmware nên bám theo đúng bảng linh kiện này để đỡ phải viết lại.

## Cấu trúc repo

```
PANDA/
├── README.md            # File này
├── PROJECT.md            # Nhật ký/kế hoạch cá nhân của Khoa (firmware)
├── ai.md                  # Tóm tắt phần AI/backend (Panda-Robotics-Client-/) — MQTT contract, biểu cảm OLED, lưu ý tích hợp
├── CLAUDE.md              # Hướng dẫn cho Claude Code khi làm việc trong repo
├── .claude/               # Cấu hình Claude Code (permissions...)
├── firmware-demo/         # Demo firmware mô phỏng (Wokwi) — ESP32 + OLED + nút bấm + MQTT
│   ├── platformio.ini
│   ├── wokwi.toml
│   ├── diagram.json
│   ├── src/
│   │   ├── main.cpp        # setup()/loop(), điều phối các module bên dưới
│   │   ├── pins.h           # định nghĩa chân GPIO tập trung
│   │   ├── display.h/.cpp   # OLED — 14 biểu cảm (đồng bộ với panda_firmware.ino của nhóm AI) + từ vựng
│   │   ├── input.h/.cpp     # đọc 2 nút bấm (debounce + edge detect)
│   │   ├── network.h/.cpp   # WiFi + MQTT (connect/reconnect/publish)
│   │   └── audio_i2s.h/.cpp # học I2S (mic INMP441) — xem WEEKLY_LOGIC.md
│   └── web-dashboard/index.html
└── Panda-Robotics-Client-/ # Phần AI/backend + dashboard web (bạn AI trong nhóm làm) — tóm tắt ở ai.md
    ├── server/              # brain.py, voice.py (STT), llm.py, tts.py, vision.py (CV), topics.py, mqtt_bridge.py
    ├── web/                 # dashboard Node/Express + Socket.IO (OLED ảo, chat log)
    ├── firmware/panda_firmware/panda_firmware.ino  # firmware mẫu ESP32 của nhóm AI (nguồn gốc 14 biểu cảm OLED ở trên)
    └── config/settings.py   # hằng số dùng chung: MQTT topics, model, ngưỡng VAD...
```

## Chạy thử demo firmware (không cần phần cứng thật)

1. Cài VS Code + extension **PlatformIO IDE** + **Wokwi for VS Code**.
2. Mở thư mục `firmware-demo/` trong VS Code.
3. Build: `pio run` (kiểm tra code compile sạch).
4. Nhấn `F1` → `Wokwi: Start Simulator` (hoặc mở `wokwi.com`, kéo thả các file trong `firmware-demo/` vào) để chạy mô phỏng ESP32 + OLED + 2 nút bấm.
5. Mở `firmware-demo/web-dashboard/index.html` bằng trình duyệt để xem bảng tiến độ realtime nhận qua MQTT.

Chi tiết kế hoạch từng bước xem [PROJECT.md](PROJECT.md).
