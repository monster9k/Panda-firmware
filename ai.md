# AI.md — Tóm tắt phần AI/Backend (`Panda-Robotics-Client-/`)

> Tổng hợp bởi Claude sau khi đọc code trong `Panda-Robotics-Client-/` (phần bạn AI trong nhóm làm). Mục đích: để Khoa (firmware) hiểu đủ phần AI để tích hợp đúng, không cần đọc lại toàn bộ ~5200 dòng code. Bỏ qua các file vá lỗi/debug một lần (`patch_*.py`, `fix_*.py`, `test_*.py`, `tools/`, `start.bat`/`stop.bat`) vì không phải kiến trúc chính thức.

## 1. Tổng quan

`Panda-Robotics-Client-/` là bộ **não AI + dashboard web** của robot Panda, chạy trên máy tính/VPS (không chạy trên ESP32). Đây là bản đầy đủ, tham vọng hơn nhiều so với mô tả "thu hẹp phạm vi: tiếng Anh một chiều" trong `PROJECT.md` — xem mục 6 "Lưu ý quan trọng" bên dưới.

Tính năng chính (theo README gốc của thư mục):
- Wake-word "Panda" (tiếng Việt/Anh), STT qua Groq Whisper, LLM trả lời (Groq/DeepSeek), TTS qua Fish Audio.
- Nhận diện khuôn mặt chủ nhân + cảm xúc + tư thế (OpenCV YuNet/SFace/FER + YOLOv8-pose).
- Phân loại 24 chủ đề câu hỏi (time/weather/math/emotion/...) → icon riêng trên OLED.
- Mô phỏng biểu cảm khuôn mặt kiểu robot **Vector (Anki)**: 11 cảm xúc + 4 trạng thái AI (questioning/hearing/thinking/speaking) + 25 hành vi idle tự chủ.
- Kiến trúc tách rời qua MQTT: não (Python) ↔ dashboard (web) ↔ robot (ESP32) không phụ thuộc nhau — **robot thật chỉ là "thin client"**, mọi tính toán AI nặng chạy trên máy tính/cloud.

## 2. Kiến trúc pipeline

```mermaid
flowchart LR
    MIC["Mic (laptop hoặc mic ESP32 qua MQTT)"] --> VOICE["voice.py: VAD + wake-word + Groq Whisper STT"]
    CAM["Camera (webcam hoặc ESP32-CAM)"] --> VISION["vision.py: YuNet + SFace + FER + YOLOv8-pose"]

    VOICE --> BRAIN["brain.py: state machine trung tâm\n(IDLE/standby/listening/thinking/speaking)"]
    VISION --> BRAIN

    BRAIN --> LLM["llm.py: Groq / DeepSeek"]
    BRAIN --> TOPICS["topics.py: phân loại 24 chủ đề (keyword, 0ms)"]
    LLM --> BRAIN
    BRAIN --> TTS["tts.py: Fish Audio (streaming từng câu)"]

    BRAIN -->|mqtt_bridge.publish| BROKER["MQTT Broker (mosquitto, localhost:1883)"]
    BROKER --> WEBDASH["web/ dashboard (OLED ảo bằng CSS + chat log)"]
    BROKER --> ROBOT["ESP32 thật hoặc virtual_robot.py (console)"]
```

## 3. Cấu trúc thư mục (đã lọc file rác/debug)

```
Panda-Robotics-Client-/
├── config/
│   ├── settings.py          # MỌI hằng số: MQTT topics, model, ngưỡng VAD, wake-word list...
│   └── secrets.example.py   # mẫu API key (secrets.py thật bị gitignore)
├── server/
│   ├── brain.py       (788 dòng) # state machine trung tâm, điều phối toàn bộ pipeline
│   ├── voice.py        (866 dòng) # VAD streaming + wake-word 3 lớp + Groq Whisper STT
│   ├── llm.py           (365 dòng) # gọi Groq/DeepSeek, stream câu trả lời
│   ├── tts.py           (476 dòng) # Fish Audio TTS streaming
│   ├── vision.py        (218 dòng) # face/emotion/pose detection (OpenCV + YOLOv8)
│   ├── topics.py         (86 dòng) # phân loại 24 chủ đề bằng keyword
│   ├── wakeword.py      (123 dòng) # Porcupine on-device wake-word (tùy chọn) + fallback Whisper
│   ├── mqtt_bridge.py     (29 dòng) # wrapper publish() MQTT, dùng chung cho mọi module
│   └── virtual_robot.py   (79 dòng) # robot ảo CHỈ IN RA CONSOLE (subscribe panda/cmd/#) — để test brain không cần chip lẫn OLED ảo
├── web/
│   ├── server.js         # Express + Socket.IO, cầu nối MQTT ↔ trình duyệt (broker mqtt://localhost:1883)
│   └── public/
│       ├── index.html, style.css
│       └── app.js (575 dòng) # dashboard: OLED ảo (CSS class-based, KHÔNG phải canvas/pixel), chat log, push-to-talk mic
└── firmware/panda_firmware/
    └── panda_firmware.ino (215 dòng) # ⭐ firmware ESP32 THẬT của nhóm AI — xem mục 5
```

## 4. MQTT — hợp đồng giao tiếp (đọc từ `config/settings.py` + `server/brain.py`)

| Topic | Hướng | Payload | Ghi chú |
|---|---|---|---|
| `panda/cmd/move` | brain → robot | `"forward"\|"back"\|"left"\|"right"\|"stop"` | điều khiển động cơ |
| `panda/cmd/arm` | brain → robot | `"wave"\|"up"\|"down"` | robot hiện KHÔNG có tay — firmware chỉ log, không lỗi |
| `panda/cmd/buzz` | brain → robot | `"on"\|"off"` | |
| `panda/cmd/face` | brain → robot + dashboard | tên biểu cảm, vd `"happy"` | xem đủ danh sách ở mục 5 |
| `panda/cmd/text` | brain → robot | chuỗi text tự do | hiện OLED chữ tĩnh |
| `panda/status` | robot → brain | `{"dist": <cm>, "btn": 0|1}` | sonar + nút, publish ~2Hz |
| `panda/ai/state` | brain → dashboard | `"IDLE"\|...` | state máy trạng thái chính |
| `panda/ai/thinking`, `panda/ai/response`, `panda/ai/topic` | brain → dashboard | text | hiện chat log + caption chủ đề |
| `panda/ai/clip`, `panda/ai/mic_live`, `panda/ai/voice_audio` | mic trình duyệt/robot → brain | audio (base64/PCM) | đường mic thay thế khi không có mic cục bộ |
| `panda/camera` | camera → vision | frame ảnh | |

**Broker**: `config/settings.py` đặt `MQTT_BROKER = "localhost"` — nhóm AI giả định **tự host Mosquitto** (đúng khoản "Não cloud (giờ G)" trong BOM), khác với demo học tập của Khoa hiện đang dùng broker công cộng `broker.hivemq.com`. Topic namespace cũng khác hẳn: `panda/cmd/*`, `panda/ai/*`, `panda/status` (chuẩn thật) so với `panda/demo/khoa/*` (chỉ dùng riêng cho demo học — theo đúng quy ước "không đụng namespace người khác" trong `CLAUDE.md`) — **không bị đụng độ, nhưng khi tích hợp thật sẽ cần chuyển firmware của Khoa sang đúng namespace `panda/cmd/*`/`panda/status`.**

## 5. OLED biểu cảm — nguồn tham chiếu chính cho việc làm biểu cảm nhúng

Nhóm AI đã làm biểu cảm ở **2 nơi khác nhau**, ngoại hình không giống hệt nhau:

1. **`web/public/app.js`** (dashboard trình duyệt) — OLED ảo dựng bằng **CSS class** (không phải vẽ pixel), danh sách đầy đủ:
   - 11 cảm xúc (`ALL_EMOTIONS`): `neutral, happy, sad, surprised, angry, love, wink, sleepy, dizzy, cool, cute`
   - 4 trạng thái AI (`ALL_AI_MODES`): `questioning, hearing, ai-thinking, speaking`
   - 25 hành vi idle tự chủ (`IDLE_BEHAVIORS`, vd `idle-look-left/right/up`, ...) — Panda tự "liếc mắt" khi rảnh, không nhại cảm xúc người dùng (mặc định `EMOTION_MIRROR = False` trong settings.py).
   - Vì dùng CSS/HTML nên **không thể copy trực tiếp sang OLED đơn sắc 128×64** — chỉ dùng để tham khảo Ý TƯỞNG (khi nào hiện biểu cảm gì).

2. **`firmware/panda_firmware/panda_firmware.ino`** — ⭐ **đây mới là code dùng được trực tiếp**: hàm `drawFace()` vẽ **11 biểu cảm bằng Adafruit_GFX/SSD1306 thật** (cùng thư viện `firmware-demo/` của Khoa đang dùng): `neutral, happy, sad, angry, surprised, sleepy, wink, love, cool, cute, dizzy`, cộng thêm `questioning` (dấu `?`) và 2 biểu cảm động `thinking` (3 chấm nhấp nháy tuần tự) / `speaking` (equalizer bar nhảy theo `millis()`).
   - Toạ độ mắt cố định: `L=32, R=76, Y=17, W=20, H=30` (không đổi giữa các firmware — nên giữ nguyên nếu Khoa port sang, để 2 phần cứng "nói cùng ngôn ngữ hình ảnh").
   - Cách chọn biểu cảm: nhận lệnh qua topic `panda/cmd/face` (MQTT thật) HOẶC gõ tay `face happy` qua Serial Monitor (chế độ mô phỏng Wokwi, không cần MQTT) — cùng 1 hàm `handleCmd()` xử lý cả 2 nguồn.

**Kế hoạch tích hợp vào `firmware-demo/` của Khoa (Tuần 3)**: port `drawFace()` từ `panda_firmware.ino` vào `firmware-demo/src/display.*`, giữ nguyên toạ độ mắt + tên biểu cảm để tương thích ngược khi ghép MQTT thật sau này. Xem `WEEKLY_LOGIC.md` mục Tuần 3 để biết chi tiết đã triển khai.

## 6. Firmware mẫu của nhóm AI vs. `firmware-demo/` của Khoa — khác biệt cần lưu ý

| | `firmware/panda_firmware.ino` (nhóm AI) | `firmware-demo/src/` (Khoa) |
|---|---|---|
| Cấu trúc | 1 file `.ino` duy nhất, Arduino IDE style | Tách nhiều file `.h`/`.cpp`, PlatformIO |
| Phạm vi | Gộp cả HEAD (OLED) + BODY (motor, HC-SR04, buzzer, nút) trong 1 sketch | Chỉ OLED + 2 nút + WiFi/MQTT + module học I2S |
| Kết nối | Mặc định chạy Serial-command (không cần MQTT); `#define USE_MQTT` để bật WiFi+MQTT thật | Luôn dùng WiFi+MQTT (broker công cộng để demo) |
| MQTT reconnect | `if (!mqtt.connected()) mqtt.connect(...)` — kiểm tra mỗi vòng loop, không có cơ chế chờ giữa các lần thử | Đã làm "bền hơn" ở Tuần 3: chỉ thử lại mỗi 5s bằng `millis()`, không chặn `loop()` (xem `WEEKLY_LOGIC.md`) |

**⚠️ Trùng chân GPIO cần lưu ý khi ghép code thật:** file `.ino` của nhóm AI dùng `PWMA=25, AIN1=26, PWMB=32, BIN1=33, POT(sim)=34` cho motor/cảm biến (thuộc BODY), trong khi `firmware-demo/src/pins.h` của Khoa đang dùng đúng **những GPIO số hiệu này** (25/26 cho 2 nút, 32/33/34 cho I2S) trên **cùng 1 board demo**. Đây không phải lỗi — theo đúng kiến trúc BOM thật, 2 bộ chân này thuộc **2 board ESP32 khác nhau** (HEAD dùng I2S, BODY dùng motor) nên khi tách board thật (Tuần 9) sẽ không đụng nhau. Chỉ cần lưu ý: **không được gộp 2 sketch này chạy chung 1 board** mà không đổi số chân.

## 7. Model/asset cần tải riêng (không có trong repo)

`face_detection_yunet_2023mar.onnx`, `face_recognition_sface_2021dec.onnx`, `emotion-ferplus-8.onnx`, `res10_300x300_ssd_iter_140000.caffemodel`, `yolov8n(-pose).pt` — tất cả bị gitignore (nặng/bản quyền), phải tự tải theo README gốc trong `Panda-Robotics-Client-/README.md` nếu muốn chạy server AI. **Không liên quan trực tiếp tới phần việc firmware của Khoa** — chỉ cần biết để hiểu vì sao thư mục `server/` thiếu vài file `.onnx`/`.pt`/`.xml` khi mở lên.

## 8. Lưu ý quan trọng — đối chiếu với "Quyết định thu hẹp phạm vi" trong `PROJECT.md`

`PROJECT.md` ghi nhận nhóm đã **thu hẹp phạm vi xuống dạy tiếng Anh một chiều, không hội thoại hai chiều** (lý do: hạn chế thời gian/kỹ năng ML). Nhưng code thực tế trong `Panda-Robotics-Client-/` cho thấy bạn AI đã build **một trợ lý hội thoại tiếng Việt đầy đủ**: wake-word tiếng Việt, LLM trả lời tự do 24 chủ đề (không chỉ chấm từ vựng), nhận diện khuôn mặt/cảm xúc/tư thế — tức là **phạm vi thực tế đã rộng hơn nhiều** so với quyết định thu hẹp đã ghi.

Đây chỉ là **quan sát khách quan từ code**, Claude không tự sửa lại phần "Quyết định thu hẹp phạm vi" trong `PROJECT.md` vì đây là quyết định của cả nhóm, không phải điều Khoa (hay Claude) có thể tự quyết một mình. **Khoa nên trao đổi lại với nhóm** xem có nên cập nhật lại phần "Scope note" trong `README.md`/`CLAUDE.md`/`PROJECT.md` cho khớp thực tế hay không, để tránh ghi sai trong báo cáo cuối kỳ.

## 9. Việc cần phối hợp giữa Khoa (firmware) và bạn AI

- **Namespace MQTT**: khi ghép thật, đổi `firmware-demo/` từ `panda/demo/khoa/*` sang đúng chuẩn `panda/cmd/*` (nhận lệnh) + `panda/status` (gửi cảm biến) mà `brain.py`/`web/server.js` đang subscribe sẵn.
- **Broker**: đổi từ `broker.hivemq.com` (demo công cộng) sang broker tự host mà nhóm AI dùng (`localhost:1883`, hoặc VPS thật khi có).
- **Biểu cảm OLED**: dùng chung 1 bộ tên biểu cảm (`neutral/happy/sad/angry/surprised/sleepy/wink/love/cool/cute/dizzy/questioning/thinking/speaking`) giữa firmware của Khoa và payload `panda/cmd/face` mà brain.py publish — đã bắt đầu port ở Tuần 3, xem `WEEKLY_LOGIC.md`.
- **Chân GPIO**: thống nhất lại bảng chân giữa 2 board HEAD/BODY thật trước khi ai đó nạp cả 2 sketch lên cùng 1 board test nhanh (xem mục 6).
