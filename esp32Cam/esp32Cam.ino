#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>

// const char *ssid = "Viet";
// const char *password = "0902042212";
const char *ssid = "zzz";
const char *password = "123456789";
const char* serverNameHardwareApi = "http://192.168.120.9:8000/test/";
const char* esp8266_url = "http://192.168.120.120/capture-complete";

WebServer server(80);


#define FLASH_LED_PIN 4  

#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27

#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    21
#define Y4_GPIO_NUM    19
#define Y3_GPIO_NUM    18
#define Y2_GPIO_NUM    5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");
  Serial.print("Address: ");
  Serial.println(WiFi.localIP());

  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;


  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Bắt đầu server
  server.on("/capture", HTTP_GET, capture);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void capture() {

  if (WiFi.status() == WL_CONNECTED) {
    for (int i = 0; i < 3; i++) {  // Gửi 3 frame
      camera_fb_t * fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        return;
      }

      HTTPClient http;
      // String urlPost = String(serverName) + "upload";
      String urlPost = String(serverNameHardwareApi) + "upload";
      http.begin(urlPost);
      http.addHeader("Content-Type", "image/jpeg");
      http.addHeader("Content-Disposition", "attachment; filename=image.jpg");
      http.addHeader("Content-Length", String(fb->len));

      int httpResponseCode = http.POST(fb->buf, fb->len);
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
      } else {
        Serial.printf("Error on sending POST: %d\n", httpResponseCode);
      }
      http.end();
      esp_camera_fb_return(fb);
      delay(100);
    }
    
    server.send(200, "text/plain", "Capture complete");
    notifyESP8266();
  }
}



void notifyESP8266(){
  HTTPClient http;
  
  http.begin(esp8266_url);  // Gửi yêu cầu HTTP tới ESP8266
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    Serial.println("Notification sent to ESP8266.");
  } else {
    Serial.printf("Error sending notification: %d\n", httpResponseCode);
  }
  http.end();
}



