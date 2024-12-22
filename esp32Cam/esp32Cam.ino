#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ArduinoJson.h> 
// const char* ssid = "zzz";
// const char* password = "123456789";

const char* ssid = "BIETTO";
const char* password = "12345678";

IPAddress local_IP(192, 168, 137, 69);  // Địa chỉ IP tĩnh
IPAddress gateway(192, 168, 137, 1);    // Địa chỉ gateway (IP của máy phát)
IPAddress subnet(255, 255, 255, 0);     // Subnet mask


const char* serverNameHardwareApi = "http://192.168.137.99:8000/garbage/upload";
const char* esp8266_url = "http://192.168.137.96:9090/capture-complete";
const char* serverNameHardwareApiCheckEmpty = "http://192.168.137.99:8000/object/upload";

WebServer server(8888);


// #define FLASH_LED_PIN 4

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

bool isCapturing = true;
void setup() {
  Serial.begin(115200);
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Cấu hình IP tĩnh thất bại!");
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");
  Serial.print("Address: ");
  Serial.println(WiFi.localIP());

  // pinMode(FLASH_LED_PIN, OUTPUT);
  // digitalWrite(FLASH_LED_PIN, HIGH);

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
    // config.fb_count = 2;
    config.fb_count = 1;
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
  // server.on("/capture", HTTP_GET, capture);
  server.on("/enableCapture", HTTP_GET, []() {
    isCapturing = true;
    delay(300);
    Serial.println("set true");
    server.send(200, "text/plain", "esp32-cam >>> ok dang chup tiep ");
  });
  server.begin();
  Serial.println("HTTP server started");
}


void loop() {
  server.handleClient();
  static unsigned long previousMillis = 0;
  const long interval = 3000;
  unsigned long currentMillis = millis();


  if (currentMillis - previousMillis >= interval && isCapturing) {
    previousMillis = currentMillis;
    bool empty = isTrashBinEmpty();

    // Gọi API kiểm tra trạng thái thùng rác
    if (empty) {
      Serial.println("xxxx khong co vat");
    } else {
      isCapturing = false;

      Serial.println("ok co vat");
      captureAndSend();
      notifyESP8266();
      //set false k cho chup nua
    }
  }
}

// Hàm kiểm tra trạng thái thùng rác qua API
bool isTrashBinEmpty() {
  Serial.println("Check co vat hay khong, dang doi ket noi....");
  if (WiFi.status() == WL_CONNECTED) {
    // Chụp ảnh
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb) {
      esp_camera_fb_return(fb);  // Giải phóng bộ đệm cũ
    }
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return true;  // Mặc định coi là trống nếu không chụp được
    }

    // Gửi ảnh lên server
    HTTPClient http;
    http.begin(serverNameHardwareApiCheckEmpty);  // URL API kiểm tra
    http.addHeader("Content-Type", "image/jpeg");
    http.addHeader("Content-Disposition", "attachment; filename=image.jpg");
    http.addHeader("Content-Length", String(fb->len));
    int httpResponseCode = http.POST(fb->buf, fb->len);
    esp_camera_fb_return(fb);  // Trả lại bộ đệm sau khi gửi
    Serial.println("da goi api: " + httpResponseCode);
    if (httpResponseCode > 0) {
      // String response = http.getString(); // Đọc phản hồi từ server
      String response = http.getString();
      // Serial.println("Trash bin status: " + response);
      // if (response == "0") {
      //   return true;
      // } else if (response == "1") {
      //   return false;
      // }

      // Phân tích chuỗi JSON
      StaticJsonDocument<200> doc;  // Kích thước bộ đệm (tùy chỉnh nếu cần)
      DeserializationError error = deserializeJson(doc, response);

      if (error) {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
        http.end();
        return true;  // Giả định thùng rác trống nếu phân tích lỗi
      }

      // Lấy giá trị từ JSON
      int status = doc["status"];  // Truy cập vào trường `status`
      Serial.println("Trash bin status (parsed): " + String(status));

      http.end();
      return status == 0;  // `true` nếu status = 0, ngược lại `false`

    } else {
      Serial.printf("Error sending image: %d\n", httpResponseCode);
      Serial.println(http.errorToString(httpResponseCode).c_str());
    }
    // esp_camera_fb_return(fb);
    http.end();
  }
  return true;  // Mặc định coi là trống nếu không kết nối được WiFi
}

void captureAndSend() {
  if (WiFi.status() == WL_CONNECTED) {
    for (int i = 0; i < 3; i++) {  // Gửi 3 frame
      camera_fb_t* fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        return;
      }

      HTTPClient http;
      // String urlPost = String(serverNameHardwareApi) + "upload";
      http.begin(serverNameHardwareApi);
      http.addHeader("Content-Type", "image/jpeg");
      http.addHeader("Content-Disposition", "attachment; filename=image.jpg");
      http.addHeader("Content-Length", String(fb->len));

      int httpResponseCode = http.POST(fb->buf, fb->len);
      esp_camera_fb_return(fb);
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
      } else {
        Serial.printf("Error on sending POST: %d\n", httpResponseCode);
        Serial.println(http.errorToString(httpResponseCode).c_str());
      }

      http.end();
      delay(100);
    }
    server.send(200, "text/plain", "Capture complete");
  }
}



void notifyESP8266() {

  if (WiFi.status() != WL_CONNECTED) return;
  WiFiClient client;
  HTTPClient http;
  http.begin(client, esp8266_url);  // Gửi yêu cầu HTTP tới ESP8266
  http.setTimeout(5000);
  int httpResponseCode = http.GET();
  Serial.println("da thong bao cho ESP8266 la chup xong");
  if (httpResponseCode > 0) {
    Serial.println("esp8266 >>> " + http.getString());
  } else {
    Serial.printf("Error sending notification: %d\n", httpResponseCode);
    Serial.println(http.errorToString(httpResponseCode).c_str());
    Serial.println(">>> line 228");
  }
  http.end();
}
