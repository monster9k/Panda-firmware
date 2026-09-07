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
const char *DEMO_WORD = "hello";

static int correctCount = 0;
static int wrongCount = 0;

static void publishResult(const char *result)
{
  char payload[128];
  snprintf(payload, sizeof(payload),
           "{\"word\":\"%s\",\"result\":\"%s\",\"ts\":%lu}",
           DEMO_WORD, result, millis());
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

// Gõ "face <ten>" qua Serial Monitor để xem thẳng 1 biểu cảm bất kỳ — cùng quy ước
// "face X" với firmware của bạn AI trong nhóm (xem ai.md mục 5). Tiện khi cần chụp ảnh
// hoặc quay video đúng một biểu cảm, khỏi phải ngồi đợi vòng tự diễn chạy tới.
static void handleSerialCommand()
{
  if (!Serial.available())
  {
    return;
  }
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (!line.startsWith("face "))
  {
    return;
  }

  String arg = line.substring(5);
  arg.trim();
  markUserAction();
  displaySetExpression(arg.c_str());
}

void setup()
{
  Serial.begin(115200);
  inputSetup();
  displaySetup();
  networkSetup();
  audioI2sSetup(); // module học I2S của Tuần 3 — xem audio_i2s.cpp để hiểu vì sao chưa đọc được âm thanh thật trong Wokwi

  displaySetStats(DEMO_WORD, correctCount, wrongCount);
  displaySetExpression("neutral");
  lastUserActionMs = millis();

  Serial.println("[main] San sang. Nut XANH = dung, nut DO = sai.");
  Serial.println("[main] De yen ~6s robot se tu dien lan luot cac bieu cam.");
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
    displaySetStats(DEMO_WORD, correctCount, wrongCount);
    publishResult("correct");
  }
  else if (event == ButtonEvent::Wrong)
  {
    markUserAction();
    wrongCount++;
    displaySetExpression("sad");
    displaySetStats(DEMO_WORD, correctCount, wrongCount);
    publishResult("wrong");
  }

  handleIdleFaces();
  displayLoop();
  audioI2sLoop();
}
