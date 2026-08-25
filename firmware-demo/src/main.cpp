#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Wokwi cung cấp sẵn 1 mạng WiFi ảo có Internet tên "Wokwi-GUEST", không mật khẩu.
const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PASSWORD = "";

// Broker MQTT công cộng, dùng để demo, không cần cài server riêng.
const char *MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;
const char *MQTT_CLIENT_ID = "panda-demo-khoa";
const char *TOPIC_RESULT = "panda/demo/khoa/result";
const char *TOPIC_PROGRESS = "panda/demo/khoa/progress";

const char *DEMO_WORD = "hello";

const int PIN_BTN_CORRECT = 25;
const int PIN_BTN_WRONG = 26;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiClient espClient;
PubSubClient mqtt(espClient);

int correctCount = 0;
int wrongCount = 0;

// INPUT_PULLUP: chân đọc là HIGH khi không bấm, LOW khi bấm (nút nối xuống GND).
// So sánh với trạng thái lần trước để chỉ xử lý đúng 1 lần mỗi lượt bấm (tránh bắn nhiều sự kiện).
bool lastCorrectState = HIGH;
bool lastWrongState = HIGH;

void connectWiFi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Dang ket noi WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" OK");
}

void connectMQTT()
{
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  while (!mqtt.connected())
  {
    Serial.print("Dang ket noi MQTT broker...");
    if (mqtt.connect(MQTT_CLIENT_ID))
    {
      Serial.println(" OK");
    }
    else
    {
      Serial.print(" that bai, rc=");
      Serial.print(mqtt.state());
      Serial.println(" -> thu lai sau 1s");
      delay(1000);
    }
  }
}

void drawFace(bool happy)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(DEMO_WORD);

  int eyeY = 35;
  if (happy)
  {
    display.fillCircle(40, eyeY, 8, SSD1306_WHITE);
    display.fillCircle(88, eyeY, 8, SSD1306_WHITE);
  }
  else
  {
    display.drawLine(32, eyeY + 5, 48, eyeY - 5, SSD1306_WHITE);
    display.drawLine(80, eyeY - 5, 96, eyeY + 5, SSD1306_WHITE);
  }

  display.setCursor(0, 55);
  display.print("Dung: ");
  display.print(correctCount);
  display.print("  Sai: ");
  display.print(wrongCount);
  display.display();
}

void publishResult(const char *result)
{
  char payload[128];
  snprintf(payload, sizeof(payload),
           "{\"word\":\"%s\",\"result\":\"%s\",\"ts\":%lu}",
           DEMO_WORD, result, millis());
  mqtt.publish(TOPIC_RESULT, payload);

  char progress[64];
  snprintf(progress, sizeof(progress),
           "{\"correct\":%d,\"wrong\":%d}", correctCount, wrongCount);
  mqtt.publish(TOPIC_PROGRESS, progress);

  Serial.print("Da publish: ");
  Serial.println(payload);
}

void setup()
{
  Serial.begin(115200);
  pinMode(PIN_BTN_CORRECT, INPUT_PULLUP);
  pinMode(PIN_BTN_WRONG, INPUT_PULLUP);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("Khong tim thay man hinh OLED");
  }

  connectWiFi();
  connectMQTT();
  drawFace(true);
}

void loop()
{
  if (!mqtt.connected())
  {
    connectMQTT();
  }
  mqtt.loop();

  bool correctState = digitalRead(PIN_BTN_CORRECT);
  bool wrongState = digitalRead(PIN_BTN_WRONG);

  if (correctState == LOW && lastCorrectState == HIGH)
  {
    correctCount++;
    drawFace(true);
    publishResult("correct");
    delay(200); // chống dội phím (debounce) đơn giản
  }
  if (wrongState == LOW && lastWrongState == HIGH)
  {
    wrongCount++;
    drawFace(false);
    publishResult("wrong");
    delay(200);
  }

  lastCorrectState = correctState;
  lastWrongState = wrongState;
}
