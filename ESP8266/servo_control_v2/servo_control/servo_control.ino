#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <Wire.h>


//wifi
const char* ssid = "BIETTO";
const char* password = "12345678";

IPAddress local_IP(192, 168, 137, 96);  // Địa chỉ IP tĩnh
IPAddress gateway(192, 168, 137, 1);    // Địa chỉ gateway (IP của máy phát)
IPAddress subnet(255, 255, 255, 0);     // Subnet mask

ESP8266WebServer server(9090);

// URL
const char* serverResultURL = "http://192.168.137.99:8000/garbage/result";
const char* serverNotifyURL = "http://192.168.137.99:8000/result/upload";
const char* serverFullURL = "http://192.168.137.99:8000/result/upload/state";
const char* esp32enableCaptureURL = "http://192.168.137.69:8888/enableCapture";

// Pins
#define SDA_PIN D2  // GPIO4
#define SCL_PIN D1  // GPIO5
#define SERVO_BIN_PIN D6
#define SERVO_DOOR_PIN D5

const int DEG_DOOR_DEFAULT = 25;
const int DEG_DOOR_TARGET = 60;

// Servo
Servo servoBin;
Servo servoDoor;


// Waste type enum
enum WasteType { PAPER,
                 PLASTIC,
                 //  GLASS,
                 METAL,
                 OTHER,
                 UNKNOWN
};
WasteType currentWaste = UNKNOWN;

// Fullness data
float isFull[4] = { 0, 0, 0, 0 };
float distances[4] = { 0.0, 0.0, 0.0, 0.0 };
// #define INTERVAL 60000  // 1 phút (60.000 mili giây)
#define INTERVAL 20000  // 1 phút (60.000 mili giây)
unsigned long lastFullnessCheck = 0;

// Setup WiFi
void connectToWiFi() {
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Cấu hình IP tĩnh thất bại!");
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
  Serial.print("Address: ");
  Serial.println(WiFi.localIP());
}

String httpGet(const char* url) {
  if (WiFi.status() != WL_CONNECTED) return "";
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
  int responseCode = http.GET();
  if (responseCode > 0) {
    return http.getString();
  } else {
    Serial.printf("HTTP GET failed: %d\n", responseCode);
    Serial.println(http.errorToString(responseCode).c_str());
    return "";
  }
  http.end();
}

bool httpPost(const char* url, const String& payload) {
  if (WiFi.status() != WL_CONNECTED) return false;
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  int responseCode = http.POST(payload);
  if (responseCode > 0) {
    Serial.printf("HTTP POST success: %s\n", http.getString().c_str());
    return true;
  } else {
    Serial.printf("HTTP POST failed: %d\n", responseCode);
    Serial.println(http.errorToString(responseCode).c_str());
    return false;
  }
  http.end();
}

// Hàm gửi dữ liệu lên server
void sendFullnessToServer() {
  StaticJsonDocument<200> doc;
  // JsonArray emptiness = doc.createNestedArray("emptiness");

  // Gán giá trị từ isFull vào JSON
  doc["paper"] = isFull[0];
  doc["plastic"] = isFull[1];
  doc["metal"] = isFull[2];
  doc["other"] = isFull[3];

  String jsonString;
  serializeJson(doc, jsonString);

  Serial.print("Dữ liệu fullness gửi lên server: ");
  Serial.println(jsonString);

  // Gửi HTTP POST
  bool postSuccess = httpPost(serverFullURL, jsonString);

  if (postSuccess) {
    Serial.println("Đã gửi dữ liệu fullness lên server thành công.");
  } else {
    Serial.println("Gửi dữ liệu fullness lên server thất bại.");
  }
}

// Parse server response
WasteType parseWasteType(const String& response) {
  StaticJsonDocument<200> doc;
  if (deserializeJson(doc, response)) return UNKNOWN;
  String result = doc["result"].as<String>();
  if (result == "paper") return PAPER;
  if (result == "plastic") return PLASTIC;
  // if (result == "glass") return GLASS;
  if (result == "other") return OTHER;
  if (result == "metal") return METAL;
  return UNKNOWN;
}

// Request fullness data from Arduino
void getFullnessDataFromArduino() {
  sendCommand();
  delay(100);
  receiveDistance();
}

// Move servo
void moveServoToBin(WasteType type) {
  int angle = 0;
  switch (type) {
    case PAPER: angle = 0; break;
    case PLASTIC: angle = 60; break;
    case METAL: angle = 130; break;
    case OTHER: angle = 180; break;
    default: return;
  }
  Serial.println(">>>>>> da xoay thung rac");
  servoBin.write(angle);
  delay(1500);
  servoDoor.write(DEG_DOOR_TARGET);
  delay(2000);
  servoDoor.write(DEG_DOOR_DEFAULT);
}

// Notify server
void notifyServer(WasteType wasteType) {
  int index = 0;
  String result = "";
  StaticJsonDocument<200> doc;
  switch (wasteType) {
    case PAPER:
      index = 0;
      result = "paper";
      break;
    case PLASTIC:
      index = 1;
      result = "plastic";
      break;
    // case GLASS:
    case METAL:
      index = 2;
      result = "metal";
      // result = "glass";
      break;
    case OTHER:
      index = 3;
      result = "other";
      break;
  }

  doc["result"] = result;

  // doc["emptiness"] = distances[index];
  // doc["emptiness"] = isFull[index];

  String jsonString;
  serializeJson(doc, jsonString);

  Serial.print(">>> check noti");
  Serial.println(jsonString);

  // Gửi HTTP POST
  bool postSuccess = httpPost(serverNotifyURL, jsonString);

  // Kiểm tra kết quả gửi
  if (postSuccess) {
    Serial.println(">>> Đã gửi thông báo đến server thành công.");
  } else {
    Serial.println(">>> Gửi thông báo đến server thất bại.");
  }
}

void sendCommand() {
  Wire.beginTransmission(8);  // Kết nối với Arduino (địa chỉ 8)
  Wire.write(1);              // Gửi lệnh '1' để đo khoảng cách
  Wire.endTransmission();     // Kết thúc truyền
  Serial.println("Gửi yêu cầu đến Arduino");
}

void receiveDistance() {
  // Wire.requestFrom(8, sizeof(float));  // Yêu cầu dữ liệu từ Arduino
  // if (Wire.available() == sizeof(float)) {
  //   Wire.readBytes((char*)&distance, sizeof(float));  // Đọc dữ liệu
  //   Serial.println("Khoảng cách nhận được từ arduino: " + String(distance) + " cm");
  // } else {
  //   Serial.println("No data received");
  // }

  Wire.requestFrom(8, sizeof(isFull));  // Yêu cầu dữ liệu từ Arduino
  delay(500);
  if (Wire.available() == sizeof(isFull)) {
    Wire.readBytes((char*)&isFull, sizeof(isFull));  // Đọc dữ liệu
    // Wire.readBytes((char*)isFull, sizeof(isFull));  // Đọc dữ liệu
    Serial.println("Đã nhận dữ liệu ve do full:");
    for (int i = 0; i < 4; i++) {
      Serial.println("Thùng " + String(i + 1) + ": " + String(isFull[i]));
    }
  } else {
    Serial.println(">>>>>>> No data received");
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  connectToWiFi();

  servoBin.attach(SERVO_BIN_PIN, 500, 2400);
  servoDoor.attach(SERVO_DOOR_PIN, 500, 2400);
  servoBin.write(0);
  servoDoor.write(DEG_DOOR_DEFAULT);  // default : 30

  server.on("/capture-complete", HTTP_GET, []() {
    Serial.println("da nhan duoc yeu cau tu ESP32-CAM");
    server.send(200, "text/plain", "esp8266 >> da nhan duoc thong bao, dang phan loai ...");
    String response = httpGet(serverResultURL);
    Serial.println("Ket qua lay dc tu server: " + response);
    server.send(200, "text/plain", "Operation complete");
    currentWaste = parseWasteType(response);
    if (currentWaste != UNKNOWN) {
      moveServoToBin(currentWaste);
      //getFullnessDataFromArduino();
      Serial.println("DANG yeu cau ESP32 chup tiep");
      String res = httpGet(esp32enableCaptureURL);
      Serial.println(res);
      Serial.println("DA yeu cau ESP32 chup tiep");

      notifyServer(currentWaste);
    } else {
      Serial.println("Unknown waste type. Skipping operation.");
    }
  });
  server.begin();
}

void loop() {
  server.handleClient();

  // Kiểm tra mỗi 1 phút
  if (millis() - lastFullnessCheck >= INTERVAL) {
    lastFullnessCheck = millis();  // Cập nhật thời gian lần cuối
    getFullnessDataFromArduino();  // Lấy dữ liệu siêu âm từ Arduino
    sendFullnessToServer();        // Gửi dữ liệu lên server
  }
}
