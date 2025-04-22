#include <WiFi.h>
#include "ThingSpeak.h"

// ======= WiFi and ThingSpeak Configuration =======
const char* WIFI_NAME = "HOME";
const char* WIFI_PASSWORD = "V090117g";

unsigned long CHANNEL_ID = 2928383;
const char* WRITE_API_KEY = "1LZ5ZQLYCDST6HWL";
const char* READ_API_KEY  = "UK78QYYCSOGGBEBR";

WiFiClient client;

// ======= Pin definitions and thresholds =======
#define LDR_AMBIENT 35
#define LDR_CAR     32
#define LED_PIN     4

int nightThreshold = 1000;
int carThreshold = 1000;

unsigned long offDelayTime = 5000;  // default in ms
unsigned long activationTime = 0;
bool ledOn = false;

// Communication control (Core 0)
unsigned long lastDayNightSendTime = 0;
unsigned long carDetectionWaitStart = 0;
bool waitingToSendCar = false;

int currentAmbientValue = 0;
int currentCarValue = 0;

// ======= Setup =======
void setup() {
  Serial.begin(115200);
  pinMode(LDR_AMBIENT, INPUT);
  pinMode(LDR_CAR, INPUT);
  pinMode(LED_PIN, OUTPUT);

  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());

  ThingSpeak.begin(client);

  // Start communication task on Core 0
  xTaskCreatePinnedToCore(
    core0Handler,
    "Core0Handler",
    4096,
    NULL,
    1,
    NULL,
    0
  );
}

// ======= Main Loop (Core 1) =======
void loop() {
  currentAmbientValue = analogRead(LDR_AMBIENT);
  currentCarValue = analogRead(LDR_CAR);

  bool isNight = currentAmbientValue < nightThreshold;
  bool carDetected = currentCarValue > carThreshold;

  // Serial output
  Serial.printf("Ambient Sensor: %d | Car Sensor: %d | %s",
                currentAmbientValue, currentCarValue,
                isNight ? "Night" : "Day");
  if (isNight) {
    Serial.printf(" | %s\n", carDetected ? "Car Detected" : "No Car");
  } else {
    Serial.println();
  }

  // Turn LED ON if night and car is detected
  if (isNight && carDetected) {
    if (!ledOn) {
      digitalWrite(LED_PIN, HIGH);
      ledOn = true;
      activationTime = millis();
      carDetectionWaitStart = millis();
      waitingToSendCar = true;
      Serial.println("LED turned ON - waiting 20s to send to field 2...");
    }
  }

  // Turn LED OFF if delay time has passed
  if (ledOn && (millis() - activationTime >= offDelayTime)) {
    int carValueNow = analogRead(LDR_CAR);
    if (carValueNow > carThreshold) {
      activationTime = millis();  // Extend time
      Serial.println("Car still detected — keeping LED ON.");
    } else {
      digitalWrite(LED_PIN, LOW);
      ledOn = false;
      waitingToSendCar = false;
      Serial.println("LED turned OFF — no car detected.");
    }
  }

  delay(500);
}

// ======= Core 0 Task: Time Reading and Data Upload =======
void core0Handler(void * parameter) {
  long previousTimeValue = offDelayTime / 1000;

  while (true) {
    // Read field 3 from ThingSpeak
    long newTimeValue = ThingSpeak.readLongField(CHANNEL_ID, 3, READ_API_KEY);
    Serial.printf("[Core 0] Field 3 value read: %ld seconds\n", newTimeValue);

    if (newTimeValue > 0 && newTimeValue < 60) {
      if (newTimeValue != previousTimeValue) {
        offDelayTime = newTimeValue * 1000;
        previousTimeValue = newTimeValue;
        Serial.printf("[Core 0] Off-delay time updated: %lu ms\n", offDelayTime);
      } else {
        Serial.println("[Core 0] Same value as before. No update.");
      }
    } else {
      Serial.println("[Core 0] Invalid value or failed to read.");
    }

    // Send day/night status (field 1) every 15s
    if (millis() - lastDayNightSendTime >= 15000) {
      int status = (currentAmbientValue > nightThreshold) ? 1 : 0;
      ThingSpeak.setField(1, status);
      int res1 = ThingSpeak.writeFields(CHANNEL_ID, WRITE_API_KEY);
      Serial.println(res1 == 200 ? "[Field 1] Day/Night status sent" : "[Field 1] Failed to send status");
      lastDayNightSendTime = millis();
    }

    // Send field 2 (car detected) after 20s of LED ON
    if (waitingToSendCar && millis() - carDetectionWaitStart >= 20000) {
      ThingSpeak.setField(2, 1);
      int res2 = ThingSpeak.writeFields(CHANNEL_ID, WRITE_API_KEY);
      Serial.println(res2 == 200 ? "[Field 2] Car detection sent (20s)" : "[Field 2] Failed to send car detection");
      waitingToSendCar = false;
    }

    delay(1000); // Run every second
  }
}
