# Kế hoạch cá nhân – Firmware nhúng (Nguyễn Viết Minh Khoa)

## Bối cảnh

Vai trò được phân công: **Firmware nhúng & AI**. Thực tế mạnh về web/app, gần như chưa có nền tảng nhúng. Deadline: **demo nhỏ phần nhúng trên máy tính** (không cần board thật) vào **thứ Sáu 28/08/2026** để giảng viên hướng dẫn thấy được đang tự học và đóng góp cho nhóm.

## Tài liệu tham khảo đã chọn

1. [Espressif Arduino-ESP32 – Getting Started](https://docs.espressif.com/projects/arduino-esp32/en/latest/getting_started.html) — tài liệu chuẩn cho lập trình ESP32 bằng Arduino framework (GPIO, I2C, WiFi, PWM).
2. [Random Nerd Tutorials – ESP32 MQTT Publish/Subscribe](https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/) — thực hành publish/subscribe MQTT bằng PubSubClient, đúng cơ chế giao tiếp ESP32 ↔ Server trong đồ án.
3. (Bổ trợ) [Wokwi Docs – ESP32 Simulation](https://docs.wokwi.com/guides/esp32) — cách mô phỏng ESP32 trên máy tính không cần phần cứng.

## Công cụ sử dụng

- IDE: VS Code + extension PlatformIO IDE
- Ngôn ngữ: C/C++ (Arduino framework)
- Mô phỏng: Wokwi (extension "Wokwi for VS Code" hoặc wokwi.com trên trình duyệt)
- MQTT broker demo: `test.mosquitto.org` (công cộng, có hỗ trợ WebSocket)
- Web dashboard: HTML/JS thuần + thư viện `mqtt.js`

## Kiến trúc demo

```
[Wokwi: ESP32 + OLED SSD1306 + 2 nút bấm (Đúng/Sai)]
        |  I2C (OLED)         |  GPIO (buttons)
        v                     v
   main.cpp (PlatformIO, C++)
        | WiFi ảo Wokwi-GUEST -> Internet
        v
   MQTT publish (PubSubClient) -> test.mosquitto.org
        | topic: panda/demo/khoa/result, panda/demo/khoa/progress
        v
   web-dashboard/index.html (mqtt.js qua WebSocket, subscribe)
        -> hiển thị realtime: từ đang học, đúng/sai, tổng điểm
```

Đây là bản thu nhỏ đúng luồng ESP32 → MQTT → Web trong đề xuất PBL4, thay STT/AI thật bằng 2 nút bấm mô phỏng kết quả chấm đúng/sai. Thể hiện 3 kỹ năng lõi của vai trò firmware: GPIO, I2C/OLED, WiFi+MQTT.

## Kế hoạch theo ngày

- [ ] **T2 24/08**: Đọc 2 tài liệu tham khảo. Cài VS Code + PlatformIO + Wokwi extension. Chạy thử ví dụ "Blink" trên Wokwi để quen quy trình build → mô phỏng.
- [ ] **T3 25/08**: Học GPIO input (đọc nút bấm, debounce) + I2C OLED cơ bản. Dựng `diagram.json` Wokwi gồm ESP32 + OLED + 2 nút. Code hiển thị mắt vui/buồn theo nút bấm (chưa cần MQTT).
- [ ] **T4 26/08**: Học WiFi + MQTT trên ESP32 (PubSubClient). Kết nối WiFi ảo `Wokwi-GUEST`, publish thử 1 message lên `test.mosquitto.org`, xác nhận bằng MQTT Explorer hoặc `mosquitto_sub`.
- [ ] **T5 27/08**: Ghép toàn luồng: nút bấm → cập nhật OLED → publish MQTT kèm bộ đếm đúng/sai. Hoàn thiện `web-dashboard/index.html`.
- [ ] **T6 28/08 (sáng)**: Kiểm thử end-to-end, quay video ngắn demo, hoàn thiện mục "Đã học được gì" bên dưới để báo cáo thầy.

## Checklist kiểm thử demo

- [ ] `pio run` build sạch, không lỗi.
- [ ] OLED hiển thị đúng trạng thái mắt khi bấm từng nút trong Wokwi.
- [ ] Serial monitor log MQTT connect + publish thành công.
- [ ] `web-dashboard/index.html` nhận message realtime và cập nhật đúng/sai khi bấm nút trong Wokwi.
- [ ] Quay lại 1 video ngắn ~1-2 phút cho buổi báo cáo.

## Đã học được gì (cập nhật dần trong tuần)

- _(điền sau khi hoàn thành từng ngày — ví dụ: cách cấu hình `platformio.ini`, cách WiFi ảo Wokwi-GUEST hoạt động, cách publish JSON qua PubSubClient, v.v.)_
