#include <WiFi.h>
#include <AudioFileSourceHTTPStream.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>

#define I2S_SPEAKER_BCLK GPIO_NUM_8  // Bit clock (BCLK)
#define I2S_SPEAKER_LRC  GPIO_NUM_7  // Left/right clock (LRCK or WS)
#define I2S_SPEAKER_DIN  GPIO_NUM_9  // Data out (DIN)

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

AudioGeneratorMP3 *mp3;
AudioFileSourceHTTPStream *file;
AudioOutputI2S *out;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Try a simple streaming MP3 source
  file = new AudioFileSourceHTTPStream("http://ice1.somafm.com/u80s-128-mp3");

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
