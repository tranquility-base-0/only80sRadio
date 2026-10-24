#include <WiFi.h>
#include "config.h"

#include "AudioFileSourceHTTPStreamUA.h"
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>

#define I2S_SPEAKER_BCLK GPIO_NUM_8  // Bit clock (BCLK)
#define I2S_SPEAKER_LRC  GPIO_NUM_7  // Left/right clock (LRCK or WS)
#define I2S_SPEAKER_DIN  GPIO_NUM_9  // Data out (DIN)

AudioGeneratorMP3 *mp3;
AudioFileSourceHTTPStreamUA *file;
AudioOutputI2S *out;

static bool isPlaceholder(const char *value) {
  if (!value || value[0] == '\0') return true;
  return strncmp(value, "YOUR_", 5) == 0;
}

static void failConfig() {
  Serial.println("\nConfiguration incomplete!");
  Serial.println("Copy radio80s_esp32s2/config.h.example to config.h and replace");
  Serial.println("every YOUR_* placeholder with a real value. See the README.");
  while (true) delay(1000);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  if (isPlaceholder(WIFI_SSID) || isPlaceholder(WIFI_PASSWORD) ||
      isPlaceholder(STATION_URL) || isPlaceholder(STATION_NAME) ||
      isPlaceholder(PLAYER_USER_AGENT)) {
    failConfig();
  }

  Serial.print("Station: ");
  Serial.println(STATION_NAME);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // (url, ua) constructor: the User-Agent is copied into the stream object
  // before open() issues the first GET, and open() applies it again on
  // every reconnect request.
  file = new AudioFileSourceHTTPStreamUA(STATION_URL, PLAYER_USER_AGENT);

  out = new AudioOutputI2S();
  out->SetPinout(I2S_SPEAKER_BCLK, I2S_SPEAKER_LRC, I2S_SPEAKER_DIN);
  out->SetGain(0.8);

  mp3 = new AudioGeneratorMP3();
  mp3->begin(file, out);

  Serial.println("Streaming started...");
}

void loop() {
  if (mp3->isRunning()) {
    if (!mp3->loop()) {
      Serial.println("MP3 decoding finished.");
      mp3->stop();
    }
  } else {
    delay(1000); // idle
  }
}
