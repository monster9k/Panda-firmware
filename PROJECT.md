# Kế hoạch cá nhân – Firmware nhúng (Nguyễn Viết Minh Khoa)

## Bối cảnh

Vai trò được phân công: **Firmware nhúng & AI**. Thực tế mạnh về web/app, gần như chưa có nền tảng nhúng. Deadline: **demo nhỏ phần nhúng trên máy tính** (không cần board thật) vào **thứ Sáu 28/08/2026** để giảng viên hướng dẫn thấy được đang tự học và đóng góp cho nhóm.

## Quyết định thu hẹp phạm vi (2026-08-25)

Đề xuất PBL4 ban đầu của nhóm hướng tới robot học **tiếng Nhật hai chiều** (nhận diện giọng nói Việt→Nhật và Nhật→Việt). Nhóm đã quyết định **thu hẹp thành dạy tiếng Anh một chiều cho trẻ em** (robot phát âm mẫu tiếng Anh, trẻ lặp lại, chấm đúng/sai — không dịch thuật, không hội thoại hai chiều).

**Lý do nhóm đưa ra:** bạn phụ trách AI trong nhóm cho rằng việc dựng nhận diện giọng nói hai chiều Việt-Nhật/Nhật-Việt quá khó vì phải tải model về và tự chỉnh sửa, vượt quá khả năng hiện tại.

**Đánh giá kỹ thuật (để nhóm hiểu rõ bản chất quyết định, tránh ghi sai lý do trong báo cáo):**

- Nhận định trên **đúng một phần, sai một phần** — vấn đề nằm ở *cách tiếp cận đã chọn*, không phải ở việc "tiếng Nhật không làm được".
- Nếu dùng đúng kiến trúc mà chính sơ đồ đề xuất của nhóm vẽ ra — gọi **API STT/TTS cloud có sẵn** (Google Speech-to-Text, Whisper API, Azure/Google TTS) — thì tiếng Nhật là ngôn ngữ được hỗ trợ sẵn, chỉ cần đổi tham số `language_code` sang `ja-JP`, **không cần tải model về hay chỉnh sửa gì cả**. Độ khó ở bước này gần như ngang tiếng Anh.
- Cái thực sự khó — và đúng là khó thật với người mới học AI trong vài tuần — là khi cần **tự host model mã nguồn mở** (Whisper chạy local, wav2vec2, VITS...) để tránh chi phí API, hoặc muốn **fine-tune** model cho phát âm trẻ em/giọng vùng miền, hoặc xây bộ **kiểm tra ngữ pháp N5-N4 + hội thoại hai chiều thật sự** như sơ đồ AI pipeline mô tả (không chỉ so khớp từ đơn). Đây là bài toán NLP/ML nâng cao, việc một thành viên chưa có nền tảng ML thấy quá sức trong thời gian một đồ án học phần là hợp lý, không phải do thiếu cố gắng.
- Kết luận: quyết định thu hẹp phạm vi là **lựa chọn đúng đắn xét theo ràng buộc thời gian và kỹ năng hiện có của nhóm**, nhưng nên ghi trong báo cáo là "thu hẹp để đảm bảo chất lượng và tiến độ" thay vì "nhận diện giọng nói tiếng Nhật không khả thi" — vì điều đó không chính xác về mặt kỹ thuật và có thể bị giảng viên phản biện.

**Tác động tới phần việc firmware:** bỏ hẳn hạng mục hiển thị **chữ Hán/Kana** trên OLED (đây là phần khó nhất, cần custom bitmap font vì Adafruit_GFX không hỗ trợ sẵn) — OLED giờ chỉ cần hiển thị từ vựng tiếng Anh bằng font ASCII có sẵn, đơn giản hơn nhiều. Timeline Tuần 5 bên dưới đã được cập nhật lại cho phù hợp.

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

---

## Lộ trình theo tuần đến cuối kỳ (báo cáo thầy hàng tuần)

Giả định cuối kỳ rơi vào khoảng cuối tháng 11/2026 (~3 tháng kể từ 24/08/2026). **Chỉnh lại ngày tháng bên dưới nếu lịch thực tế của môn khác đi.** Nguyên tắc: toàn bộ phần việc firmware phải xong ở Tuần 13, 2 tuần cuối (14–15) chỉ dùng để hoàn thiện tài liệu/demo và dự phòng rủi ro, không phát triển tính năng mới.

Đánh số tuần tiếp nối đúng theo mốc "Tuần 2" mà nhóm đã thống nhất trong đề xuất PBL4 (Tuần 2 = 24/08–30/08).

| Tuần | Thời gian | Nội dung chính (Firmware nhúng) | Báo cáo thầy tuần này |
|---|---|---|---|
| 2 | 24/08–30/08 | Demo nhỏ: ESP32 mô phỏng (Wokwi) + OLED + 2 nút bấm + MQTT publish + web dashboard (xem chi tiết ở mục Kế hoạch theo ngày). | Video demo luồng ESP32→MQTT→Web + 2 tài liệu tham khảo đã đọc. |
| 3 | 31/08–06/09 | Học I2S/thu âm cơ bản (mô phỏng); tách code firmware thành các module rõ ràng (wifi/mqtt, display, input); làm MQTT reconnect logic bền hơn (không chỉ demo 1 lần). | Cấu trúc firmware mới + giải thích tại sao tách module. |
| 4 | 07/09–13/09 | Firmware **subscribe** lệnh/kết quả từ server (không chỉ publish một chiều); phối hợp với Thái để nhận kết quả fuzzy matching (từ vựng tiếng Anh) qua MQTT và hiển thị lên OLED. | Demo 2 chiều: server gửi lệnh → ESP32 phản ứng. |
| 5 | 14/09–20/09 | Hiển thị nhiều biểu cảm mắt (vui/buồn/ngạc nhiên) + từ vựng tiếng Anh trên OLED bằng font ASCII có sẵn của Adafruit_GFX (không cần custom bitmap font nữa sau khi thu hẹp phạm vi sang tiếng Anh — xem "Quyết định thu hẹp phạm vi" ở trên). Tận dụng thời gian dư ra để làm sớm layout hiển thị hình ảnh minh họa đơn giản (icon/emoji dạng bitmap) đi kèm từ vựng. | OLED hiển thị đúng từ vựng tiếng Anh + icon minh họa + đủ 3 biểu cảm. |
| 6 | 21/09–27/09 | Né vật cản bằng cảm biến siêu âm HC-SR04 (mô phỏng); **kiểm thử tích hợp toàn hệ thống đầu-cuối** — đây là mốc báo cáo giữa kỳ theo kế hoạch gốc của nhóm. | **Báo cáo tiến độ giữa kỳ**: demo toàn bộ luồng đã làm từ Tuần 2–6. |
| 7 | 28/09–04/10 | Học điều khiển động cơ DC qua L298N + PWM; code phản hồi chuyển động (xoay vòng khi đúng, lắc khi sai). | Demo motor phản hồi theo kết quả đúng/sai. |
| 8 | 05/10–11/10 | Tích hợp điều khiển di chuyển bằng lệnh giọng nói cơ bản (server gửi lệnh "tiến"/"dừng" qua MQTT → ESP32 điều khiển motor). | Demo điều khiển di chuyển bằng lệnh MQTT giả lập giọng nói. |
| 9 | 12/10–18/10 | Nếu nhóm đã có phần cứng thật: chuyển từ Wokwi sang board thật, lắp ráp, hiệu chỉnh motor/cảm biến thật. Nếu chưa có: tiếp tục hoàn thiện mô phỏng + viết tài liệu chuẩn bị chuyển sang thật. | Ảnh/video phần cứng thật (nếu có) hoặc báo cáo rủi ro nếu chưa có board. |
| 10 | 19/10–25/10 | Làm cứng cáp firmware: xử lý mất WiFi/MQTT giữa chừng, watchdog, xử lý lỗi cảm biến/OLED; dọn code + comment lại. | So sánh trước/sau khi thêm xử lý lỗi (demo ngắt mạng giữa chừng vẫn tự hồi phục). |
| 11 | 26/10–01/11 | Tích hợp end-to-end với AI/Server thật của nhóm (STT/TTS/fuzzy matching thật thay cho nút bấm giả lập) — phối hợp chặt với Thái. | Demo học từ vựng thật bằng giọng nói (không còn nút bấm giả). |
| 12 | 02/11–08/11 | Kiểm thử toàn diện: edge case (mất mạng, dữ liệu sai định dạng, cảm biến nhiễu); sửa lỗi phát sinh. | Danh sách bug đã tìm thấy + đã sửa. |
| 13 | 09/11–15/11 | **Code freeze**: hoàn thiện tính năng cuối cùng, không thêm tính năng mới sau tuần này. | Xác nhận firmware đã hoàn chỉnh, sẵn sàng cho 2 tuần đệm. |
| 14 | 16/11–22/11 | *(Tuần đệm 1 — không code tính năng mới)* Viết tài liệu kỹ thuật phần firmware, quay video demo đầy đủ, chuẩn bị slide báo cáo. | Bản nháp tài liệu + video demo đầy đủ. |
| 15 | 23/11–29/11 | *(Tuần đệm 2 — dự phòng)* Fix bug phát sinh cuối cùng, tổng duyệt báo cáo, chuẩn bị bảo vệ đồ án. | Báo cáo/bảo vệ cuối kỳ. |

Checklist nhanh mỗi tuần (lặp lại):
- [ ] Đầu tuần: đọc lại dòng tương ứng trong bảng trên, xác nhận mục tiêu tuần.
- [ ] Giữa tuần: code + test trên Wokwi (hoặc board thật từ Tuần 9).
- [ ] Cuối tuần: cập nhật mục "Đã học được gì", chuẩn bị demo ngắn để báo cáo thầy.
