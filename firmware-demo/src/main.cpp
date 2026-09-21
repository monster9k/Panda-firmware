#include <Arduino.h>
#include "pins.h"
#include "display.h"
#include "input.h"
#include "network.h"
#include "audio_i2s.h"

// main.cpp chỉ đóng vai trò "nhạc trưởng": setup()/loop() gọi các module, không tự đụng
// vào chi tiết TFT/WiFi/MQTT/nút bấm (những phần đó nằm ở display.*, network.*, input.*).
// Xem WEEKLY_LOGIC.md để hiểu lý do tách module theo cách này.

const char *TOPIC_RESULT = "panda/demo/khoa/result";
const char *TOPIC_PROGRESS = "panda/demo/khoa/progress";

static int correctCount = 0;
static int wrongCount = 0;

// ---------------------------------------------------------------------------
//  Danh sách từ vựng demo (tiếng Anh + tiếng Nhật)
//
//  Phạm vi dự án từ 21/09/2026 có thêm tiếng Nhật bên cạnh tiếng Anh, nên mỗi từ mang 3
//  phần: chữ Latin, Kanji, và cách đọc bằng Kana. Từ nào tiếng Nhật không dùng Kanji thì
//  để trống ô kanji — display.cpp sẽ tự phóng to phần Kana thay vào chỗ đó.
//
//  LƯU Ý khi sửa file này: phải lưu bằng mã hoá UTF-8 (VS Code mặc định là UTF-8, xem góc
//  dưới bên phải). Nếu lưu nhầm bảng mã khác, các chữ Nhật bên dưới biến thành byte rác và
//  màn hình sẽ hiện ra khoảng trắng.
//
//  Đây chỉ là dữ liệu tạm để test khi chưa có server: sau này khi bật lại MQTT, từ vựng sẽ
//  do server gửi xuống qua topic panda/cmd/word và gọi đúng hàm displaySetWord().
// ---------------------------------------------------------------------------
struct VocabWord
{
  const char *english;
  const char *kanji;
  const char *kana;
};

static const VocabWord VOCAB[] = {
    {"dog", "犬", "いぬ"},
    {"cat", "猫", "ねこ"},
    {"water", "水", "みず"},
    {"fish", "魚", "さかな"},
    {"mountain", "山", "やま"},
    {"flower", "花", "はな"},
    {"book", "本", "ほん"},
    {"rain", "雨", "あめ"},
    {"cake", "", "ケーキ"}, // từ mượn, tiếng Nhật viết bằng Katakana, không có Kanji
};
static const int VOCAB_LEN = sizeof(VOCAB) / sizeof(VOCAB[0]);

static int vocabIndex = 0;

static void showVocabWord(int index)
{
  const VocabWord &w = VOCAB[index];
  displaySetWord(w.english, w.kanji, w.kana);
}

// Sang từ kế tiếp, hết danh sách thì quay vòng về đầu. Luôn gọi SAU publishResult() để
// message gửi đi còn mang đúng từ vừa được chấm, không phải từ mới.
static void nextVocabWord()
{
  vocabIndex = (vocabIndex + 1) % VOCAB_LEN;
  showVocabWord(vocabIndex);
}

static void publishResult(const char *result)
{
  const VocabWord &w = VOCAB[vocabIndex];
  char payload[160];
  snprintf(payload, sizeof(payload),
           "{\"word\":\"%s\",\"kanji\":\"%s\",\"result\":\"%s\",\"ts\":%lu}",
           w.english, w.kanji, result, millis());
  networkPublish(TOPIC_RESULT, payload);

  char progress[64];
  snprintf(progress, sizeof(progress),
           "{\"correct\":%d,\"wrong\":%d}", correctCount, wrongCount);
  networkPublish(TOPIC_PROGRESS, progress);

  Serial.print("Da publish: ");
  Serial.println(payload);
}

// ---------------------------------------------------------------------------
//  Biểu cảm lúc "rảnh"
//
//  Chỉ còn 2 nút: ĐÚNG -> happy, SAI -> sad. Nút bấm luôn được ưu tiên và giữ nguyên
//  biểu cảm đó một lúc. Nếu quá IDLE_ENTER_MS mà không ai bấm gì, robot tự lần lượt
//  diễn các biểu cảm còn lại cho đỡ "chết cứng" — cũng là cách xem hết mọi animation
//  mà không cần thêm nút test nào.
// ---------------------------------------------------------------------------
static const char *IDLE_FACES[] = {
    "neutral", "questioning", "cute", "surprised", "thinking", "love",
    "cool", "hearing", "speaking", "dizzy", "wink", "sleepy"};
static const int IDLE_FACES_LEN = sizeof(IDLE_FACES) / sizeof(IDLE_FACES[0]);

static const unsigned long IDLE_ENTER_MS = 6000; // im lặng bao lâu thì bắt đầu tự diễn
static const unsigned long IDLE_STEP_MS = 4500;  // mỗi biểu cảm giữ bao lâu

static unsigned long lastUserActionMs = 0;
static unsigned long idleStepMs = 0;
static bool idleActive = false;
static int idleIndex = 0;

// Gọi khi có tương tác của người dùng (bấm nút / gõ lệnh Serial): tạm dừng chế độ tự
// diễn và đếm lại từ đầu, để biểu cảm vừa đặt không bị ghi đè ngay sau đó.
static void markUserAction()
{
  lastUserActionMs = millis();
  idleActive = false;
}

static void handleIdleFaces()
{
  unsigned long now = millis();
  if (now - lastUserActionMs < IDLE_ENTER_MS)
  {
    return; // vẫn đang trong lúc "có người tương tác"
  }

  if (!idleActive)
  {
    idleActive = true;
    idleStepMs = now - IDLE_STEP_MS; // đổi mặt ngay lập tức khi vừa vào chế độ rảnh
  }

  if (now - idleStepMs < IDLE_STEP_MS)
  {
    return;
  }
  idleStepMs = now;
  displaySetExpression(IDLE_FACES[idleIndex]);
  idleIndex = (idleIndex + 1) % IDLE_FACES_LEN;
}

// Tách một chuỗi thành tối đa 3 phần ngăn cách bằng dấu cách. Cắt theo BYTE là an toàn với
// tiếng Nhật: trong UTF-8, byte của dấu cách (0x20) không bao giờ xuất hiện bên trong một
// chữ nhiều byte, nên không có nguy cơ cắt đôi chữ 犬 thành byte rác.
static int splitWords(const String &text, String *parts, int maxParts)
{
  int count = 0;
  int i = 0;
  while (count < maxParts && i < (int)text.length())
  {
    while (i < (int)text.length() && text[i] == ' ')
    {
      i++;
    }
    if (i >= (int)text.length())
    {
      break;
    }
    int space = text.indexOf(' ', i);
    if (space < 0)
    {
      space = text.length();
    }
    parts[count++] = text.substring(i, space);
    i = space;
  }
  return count;
}

// 2 lệnh gõ qua Serial Monitor:
//
//   face <ten>              — xem thẳng 1 biểu cảm bất kỳ (cùng quy ước với firmware của bạn
//                             AI trong nhóm, xem ai.md mục 5).
//   word <anh> <kanji> <kana>  — đặt từ vựng tuỳ ý, ví dụ: word dog 犬 いぬ
//
// Lệnh "word" là cách kiểm chứng luồng UTF-8 chạy đúng với chữ BẤT KỲ chứ không chỉ mấy từ
// dựng sẵn trong VOCAB — đúng việc mà MQTT sẽ làm sau này. Gõ tiếng Nhật vào Serial Monitor
// cần bàn phím/IME tiếng Nhật; nếu không gõ được, cứ dùng danh sách VOCAB dựng sẵn.
static void handleSerialCommand()
{
  if (!Serial.available())
  {
    return;
  }
  String line = Serial.readStringUntil('\n');
  line.trim();

  if (line.startsWith("face "))
  {
    String arg = line.substring(5);
    arg.trim();
    markUserAction();
    displaySetExpression(arg.c_str());
    return;
  }

  if (line.startsWith("word "))
  {
    String parts[3];
    int count = splitWords(line.substring(5), parts, 3);
    if (count == 0)
    {
      Serial.println("[main] Cu phap: word <tieng anh> <kanji> <kana>");
      return;
    }
    markUserAction();
    displaySetWord(parts[0].c_str(), parts[1].c_str(), parts[2].c_str());
  }
}

void setup()
{
  Serial.begin(115200);
  inputSetup();
  displaySetup();
  networkSetup();
  audioI2sSetup(); // module học I2S của Tuần 3 — xem audio_i2s.cpp để hiểu vì sao chưa đọc được âm thanh thật trong Wokwi

  showVocabWord(vocabIndex);
  displaySetScore(correctCount, wrongCount);
  displaySetExpression("neutral");
  lastUserActionMs = millis();

  Serial.println("[main] San sang. Nut XANH = dung, nut DO = sai (bam xong tu dong sang tu ke tiep).");
  Serial.println("[main] De yen ~6s robot se tu dien lan luot cac bieu cam.");
  Serial.println("[main] Go 'face <ten>' hoac 'word <anh> <kanji> <kana>' de test thu cong.");

  // In lượng RAM trống còn lại sau khi mọi thứ đã khởi tạo xong (khung đệm màn hình 62KB,
  // WiFi, MQTT...). Nếu con số này tụt xuống dưới ~40KB thì cần lo: các thao tác mạng sau
  // đó có thể xin thêm bộ nhớ không được và sinh lỗi khó hiểu.
  Serial.print("[main] RAM trong con lai: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" byte");
}

void loop()
{
  networkLoop();
  handleSerialCommand();

  ButtonEvent event = inputPoll();
  if (event == ButtonEvent::Correct)
  {
    markUserAction();
    correctCount++;
    displaySetExpression("happy");
    displaySetScore(correctCount, wrongCount);
    publishResult("correct");
    nextVocabWord();
  }
  else if (event == ButtonEvent::Wrong)
  {
    markUserAction();
    wrongCount++;
    displaySetExpression("sad");
    displaySetScore(correctCount, wrongCount);
    publishResult("wrong");
    nextVocabWord();
  }

  handleIdleFaces();
  displayLoop();
  audioI2sLoop();

  // NHƯỜNG CPU cho hệ điều hành — 1 dòng nhỏ nhưng quan trọng.
  //
  // Từ khi đổi sang màn SPI, không còn chỗ nào trong loop() "nghỉ" nữa: bản OLED cũ vô
  // tình an toàn vì hàm gửi I2C chờ bằng semaphore (lúc chờ là task tự ngủ, CPU được
  // nhường cho việc khác), còn hàm gửi SPI chờ bằng vòng lặp đọc thanh ghi liên tục,
  // không nhường gì cả. Các hàm còn lại đều return ngay. Kết quả: task chính quay
  // 100% CPU không ngừng nghỉ, khiến (1) máy tính phải mô phỏng một con CPU lúc nào cũng
  // bận tối đa nên nóng và quạt kêu to, (2) các task nền của hệ thống (WiFi, TCP/IP) bị
  // chèn ép, dễ sinh lỗi lạ.
  //
  // delay(1) KHÁC HẲN delay() dài đã học ở Bước 10: nó không phải "chờ cho hết giờ" mà là
  // báo cho hệ điều hành "tôi xong việc rồi, ai cần CPU thì dùng đi" — và 1ms là đúng 1
  // nhịp tick của FreeRTOS, tức mức nhường nhỏ nhất có thể. Vòng lặp vẫn chạy ~1000
  // lần/giây, thừa sức bắt kịp nút bấm (debounce 200ms) và khung hình (30ms).
  delay(1);
}
