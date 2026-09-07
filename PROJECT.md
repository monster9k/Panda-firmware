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

> **⚠️ Cập nhật cần lưu ý (03/09/2026)**: sau khi đọc code AI thật của nhóm trong `Panda-Robotics-Client-/` (tóm tắt ở `ai.md`), thấy bạn phụ trách AI đã build một trợ lý hội thoại tiếng Việt đầy đủ (wake-word, LLM trả lời tự do 24 chủ đề, nhận diện khuôn mặt/cảm xúc) — **rộng hơn nhiều** so với "tiếng Anh một chiều" đã ghi ở trên. Đây là quan sát từ code, chưa phải quyết định lại của nhóm — xem `ai.md` mục 8 để biết chi tiết, cần trao đổi lại với nhóm xem có cập nhật lại phần "Quyết định thu hẹp phạm vi" này không.

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
[Wokwi: ESP32 + TFT ILI9341 2.4" + 2 nút bấm (Đúng/Sai)]
        |  SPI 40MHz (TFT)    |  GPIO (buttons)
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

Đây là bản thu nhỏ đúng luồng ESP32 → MQTT → Web trong đề xuất PBL4, thay STT/AI thật bằng 2 nút bấm mô phỏng kết quả chấm đúng/sai. Thể hiện 3 kỹ năng lõi của vai trò firmware: GPIO, SPI/màn hình, WiFi+MQTT.

## Kế hoạch theo ngày

- [x] **T2 24/08**: Đọc 2 tài liệu tham khảo. Cài VS Code + PlatformIO + Wokwi extension. Chạy thử ví dụ "Blink" trên Wokwi để quen quy trình build → mô phỏng.
- [x] **T3 25/08**: Học GPIO input (đọc nút bấm, debounce) + I2C OLED cơ bản. Dựng `diagram.json` Wokwi gồm ESP32 + OLED + 2 nút. Code hiển thị mắt vui/buồn theo nút bấm (chưa cần MQTT).
- [x] **T4 26/08**: Học WiFi + MQTT trên ESP32 (PubSubClient). Kết nối WiFi ảo `Wokwi-GUEST`, publish thử 1 message lên broker (đổi sang `broker.hivemq.com` — xem ghi chú rc=-2 bên dưới).
- [x] **T5 27/08**: Ghép toàn luồng: nút bấm → cập nhật OLED → publish MQTT kèm bộ đếm đúng/sai. Hoàn thiện `web-dashboard/index.html`.
- [~] **T6 28/08 (sáng)**: Kiểm thử end-to-end xong (xem checklist dưới). **Còn thiếu: quay video ngắn demo** — việc quay màn hình là thao tác thủ công, cần tự làm khi mở Wokwi lên chạy lại.

## Checklist kiểm thử demo

- [x] `pio run` build sạch, không lỗi — xác nhận lại ngày 03/09/2026 (RAM 13.8%, Flash 59.4%, SUCCESS).
- [x] OLED hiển thị đúng trạng thái mắt khi bấm từng nút trong Wokwi.
- [x] Serial monitor log MQTT connect + publish thành công.
- [x] `web-dashboard/index.html` nhận message realtime và cập nhật đúng/sai khi bấm nút trong Wokwi.
- [ ] Quay lại 1 video ngắn ~1-2 phút cho buổi báo cáo. — **việc còn lại duy nhất của Tuần 2**, không thể tự động hoá, cần tự quay khi chạy Wokwi.

> 4 mục đầu được xác nhận dựa trên: build vừa chạy lại thành công trong phiên này, và ghi chú trong `learningprocess.md`/lịch sử code (broker đổi sang `broker.hivemq.com` sau khi gặp lỗi `rc=-2`) cho thấy toàn luồng OLED → MQTT → dashboard đã chạy thật trên Wokwi trước đó. Claude không tự mở được trình mô phỏng Wokwi (không có `wokwi-cli`/token) nên không re-run trực tiếp — nếu muốn chắc chắn 100%, mở lại Wokwi và xem qua 1 lượt trước khi quay video.

## Đã học được gì (cập nhật dần trong tuần)

- **`platformio.ini`**: `lib_deps` khai báo thư viện theo cú pháp `tác_giả/Tên Thư Viện @ ^phiên_bản` — PlatformIO tự tải từ registry, không cần cài thủ công như Arduino IDE.
- **WiFi ảo `Wokwi-GUEST`**: Wokwi cấp sẵn 1 mạng WiFi mô phỏng có Internet thật (không mật khẩu), nên `WiFi.begin()` + vòng `while (WiFi.status() != WL_CONNECTED)` hoạt động y hệt code cho board thật — không cần đổi gì khi có phần cứng.
- **`INPUT_PULLUP` đảo logic**: chân nút bấm nối xuống GND, khi không bấm chân được kéo lên HIGH nhờ điện trở pull-up nội bộ của ESP32; bấm nút = nối chân xuống GND = đọc được LOW. Vì vậy "bấm" tương ứng giá trị LOW chứ không phải HIGH.
- **Edge detection**: so sánh trạng thái đọc hiện tại với trạng thái đọc lần trước (`lastState`) để chỉ xử lý đúng 1 lần mỗi lượt bấm, tránh xử lý lặp lại liên tục khi giữ nút.
- **Debug MQTT `rc=-2`**: mã lỗi này của PubSubClient nghĩa là **kết nối TCP thất bại** (network-level), không phải sai thông tin đăng nhập/topic. Gặp lỗi này khi dùng `test.mosquitto.org` qua Wokwi (nghi do broker công khai bị quá tải/chặn từ gateway Wokwi) — khắc phục bằng cách đổi sang broker công khai khác (`broker.hivemq.com`, cổng 1883 cho firmware / 8000 cho WebSocket dashboard).
- **`mqtt.loop()` phải gọi liên tục trong `loop()`**: đây là hàm xử lý nền của PubSubClient (giữ kết nối, nhận message) — không gọi thì kết nối MQTT âm thầm bị treo dù code không báo lỗi ngay.
- Chi tiết đầy đủ hơn (kèm sơ đồ luồng) đã chuyển sang `WEEKLY_LOGIC.md` (không push git) để không làm phình file kế hoạch này.

---

## Lộ trình theo tuần đến cuối kỳ (báo cáo thầy hàng tuần)

Giả định cuối kỳ rơi vào khoảng cuối tháng 11/2026 (~3 tháng kể từ 24/08/2026). **Chỉnh lại ngày tháng bên dưới nếu lịch thực tế của môn khác đi.** Nguyên tắc: toàn bộ phần việc firmware phải xong ở Tuần 13, 2 tuần cuối (14–15) chỉ dùng để hoàn thiện tài liệu/demo và dự phòng rủi ro, không phát triển tính năng mới.

Đánh số tuần tiếp nối đúng theo mốc "Tuần 2" mà nhóm đã thống nhất trong đề xuất PBL4 (Tuần 2 = 24/08–30/08).

> **Chia việc 2 người phần nhúng (từ Tuần 4, 08/09/2026):** Thắng (vai trò "Vi điều khiển & Báo cáo") chính thức làm chung phần firmware từ tuần này. Quy tắc đọc bảng dưới: **không có tag = mặc định Khoa làm**; tag `-Thắng` = phần đó Thắng làm; tag `-Khoa & Thắng` = làm chung/phối hợp. Chi tiết đầy đủ về cách chia (theo board HEAD/BODY), quy ước đặt tên module để tránh đụng file, và cách 2 người cùng sửa `main.cpp` mà không giẫm chân nhau — xem [PHANCONG.md](PHANCONG.md).

| Tuần | Thời gian | Nội dung chính (Firmware nhúng) | Báo cáo thầy tuần này |
|---|---|---|---|
| 2 | 24/08–30/08 | Demo nhỏ: ESP32 mô phỏng (Wokwi) + OLED + 2 nút bấm + MQTT publish + web dashboard (xem chi tiết ở mục Kế hoạch theo ngày). *(Trước khi Thắng tham gia — Khoa làm một mình.)* | Video demo luồng ESP32→MQTT→Web + 2 tài liệu tham khảo đã đọc. |
| 3 | 31/08–06/09 | Học I2S/thu âm cơ bản (mô phỏng); tách code firmware thành các module rõ ràng (wifi/mqtt, display, input); làm MQTT reconnect logic bền hơn (không chỉ demo 1 lần). **Bổ sung giữa tuần (03/09)**: đọc code AI của nhóm trong `Panda-Robotics-Client-/` (tóm tắt ở `ai.md`), phát hiện bạn AI đã có sẵn firmware mẫu vẽ 11 biểu cảm OLED bằng Adafruit_GFX/SSD1306 — port trực tiếp thành 14 biểu cảm (`neutral/happy/sad/angry/surprised/sleepy/wink/love/cool/cute/dizzy/questioning/thinking/speaking`) vào `display.h/.cpp`, làm luôn trong tuần này thay vì để tới Tuần 5. *(Trước khi Thắng tham gia — Khoa làm một mình.)* | Cấu trúc firmware mới + giải thích tại sao tách module + demo 14 biểu cảm OLED qua lệnh Serial `face <ten>`. |
| 4 | 07/09–13/09 | Firmware **subscribe** lệnh/kết quả từ server (không chỉ publish một chiều); phối hợp với Thái (AI) để nhận kết quả fuzzy matching (từ vựng tiếng Anh) qua MQTT và hiển thị lên màn. Song song: tìm hiểu điều khiển động cơ DC (L298N/TB6612FNG) qua PWM + cảm biến HC-SR04 trong Wokwi, chuẩn bị cho Tuần 6–7 -Thắng. **Bổ sung đầu tuần (08/09)**: nhóm đổi linh kiện hiển thị từ 2× OLED SSD1306 sang 1× TFT màu ILI9341 2.4" SPI → chuyển toàn bộ `display.*` sang `Adafruit_ILI9341` + `GFXcanvas16`, viết lại 15 biểu cảm có animation liên tục và màu riêng theo cảm xúc, rút mạch còn 2 nút Đúng/Sai + tự luân chuyển biểu cảm khi rảnh -Khoa. | Demo 2 chiều: server gửi lệnh → ESP32 phản ứng. Kèm demo khuôn mặt màu mới trên ILI9341. |
| 5 | 14/09–20/09 | ~~Hiển thị nhiều biểu cảm mắt~~ (đã làm sớm ở Tuần 3, viết lại cho màn màu ở Tuần 4 — xem trên) — tuần này dồn hết vào hiển thị từ vựng tiếng Anh trên màn bằng font ASCII có sẵn của Adafruit_GFX (không cần custom bitmap font sau khi thu hẹp phạm vi sang tiếng Anh — xem "Quyết định thu hẹp phạm vi" ở trên) + layout hiển thị hình ảnh minh họa đơn giản (icon/emoji dạng bitmap) đi kèm từ vựng. Màn 320×240 màu rộng gấp ~9 lần OLED cũ nên có thể đặt từ vựng + icon cạnh khuôn mặt thay vì tranh chỗ. Song song: dựng thêm buzzer + 2 nút bấm BODY vào `diagram.json`, viết khung module `motor.h/.cpp` (chưa cần chạy thật) -Thắng. | Màn hiển thị đúng từ vựng tiếng Anh + icon minh họa, phối hợp bố cục với 15 biểu cảm đã có. |
| 6 | 21/09–27/09 | Né vật cản bằng cảm biến siêu âm HC-SR04 (mô phỏng) -Thắng; **kiểm thử tích hợp toàn hệ thống đầu-cuối** -Khoa & Thắng — đây là mốc báo cáo giữa kỳ theo kế hoạch gốc của nhóm. | **Báo cáo tiến độ giữa kỳ**: demo toàn bộ luồng đã làm từ Tuần 2–6. |
| 7 | 28/09–04/10 | Học điều khiển động cơ DC qua L298N/TB6612FNG + PWM; code phản hồi chuyển động (xoay vòng khi đúng, lắc khi sai) -Thắng. | Demo motor phản hồi theo kết quả đúng/sai. |
| 8 | 05/10–11/10 | Tích hợp điều khiển di chuyển bằng lệnh giọng nói cơ bản (server gửi lệnh "tiến"/"dừng" qua MQTT → ESP32 điều khiển motor) — dùng lại module `network` Khoa đã xây để nhận lệnh, Thắng viết phần xử lý motor tương ứng -Khoa & Thắng. | Demo điều khiển di chuyển bằng lệnh MQTT giả lập giọng nói. |
| 9 | 12/10–18/10 | Nếu nhóm đã có phần cứng thật: chuyển từ Wokwi sang board thật, lắp ráp, hiệu chỉnh — Khoa phụ trách board HEAD (OLED, mic, MQTT), Thắng phụ trách board BODY (motor, cảm biến, buzzer) -Khoa & Thắng. Nếu chưa có: tiếp tục hoàn thiện mô phỏng + viết tài liệu chuẩn bị chuyển sang thật. | Ảnh/video phần cứng thật (nếu có) hoặc báo cáo rủi ro nếu chưa có board. |
| 10 | 19/10–25/10 | Làm cứng cáp firmware: Khoa xử lý mất WiFi/MQTT giữa chừng, watchdog, lỗi OLED; Thắng xử lý lỗi cảm biến siêu âm/động cơ kẹt -Khoa & Thắng. Mỗi người tự dọn code + comment lại phần mình. | So sánh trước/sau khi thêm xử lý lỗi (demo ngắt mạng giữa chừng vẫn tự hồi phục). |
| 11 | 26/10–01/11 | Tích hợp end-to-end với AI/Server thật của nhóm (STT/TTS/fuzzy matching thật thay cho nút bấm giả lập) — phối hợp chặt với Thái (AI). Thắng hỗ trợ kiểm thử phản hồi động cơ theo kết quả AI thật -Khoa & Thắng. | Demo học từ vựng thật bằng giọng nói (không còn nút bấm giả). |
| 12 | 02/11–08/11 | Kiểm thử toàn diện: edge case (mất mạng, dữ liệu sai định dạng, cảm biến nhiễu); mỗi người tự kiểm thử phần mình rồi tổng hợp bug chung -Khoa & Thắng. | Danh sách bug đã tìm thấy + đã sửa. |
| 13 | 09/11–15/11 | **Code freeze**: hoàn thiện tính năng cuối cùng, không thêm tính năng mới sau tuần này -Khoa & Thắng. | Xác nhận firmware đã hoàn chỉnh, sẵn sàng cho 2 tuần đệm. |
| 14 | 16/11–22/11 | *(Tuần đệm 1 — không code tính năng mới)* Thắng chủ trì viết tài liệu kỹ thuật phần firmware + quay video demo đầy đủ + chuẩn bị slide báo cáo -Thắng (chủ trì, đúng vai trò "Báo cáo"). Khoa hỗ trợ viết tài liệu kỹ thuật cho module mình phụ trách -Khoa. | Bản nháp tài liệu + video demo đầy đủ. |
| 15 | 23/11–29/11 | *(Tuần đệm 2 — dự phòng)* Mỗi người fix bug phát sinh cuối cùng ở phần mình; Thắng chủ trì tổng duyệt báo cáo, chuẩn bị bảo vệ đồ án -Khoa & Thắng. | Báo cáo/bảo vệ cuối kỳ. |

Checklist nhanh mỗi tuần (lặp lại):
- [ ] Đầu tuần: đọc lại dòng tương ứng trong bảng trên, xác nhận mục tiêu tuần.
- [ ] Giữa tuần: code + test trên Wokwi (hoặc board thật từ Tuần 9).
- [ ] Cuối tuần: cập nhật mục "Đã học được gì", chuẩn bị demo ngắn để báo cáo thầy.
