#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h> 
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Servo.h>

// const char *ssid = "Viet";
// const char *password = "0902042212";
const char *ssid = "zzz";
const char *password = "123456789";
ESP8266WebServer server(80);

// url
String esp32cam_url_capture = "http://192.168.120.244/capture";
String serverName_getResult = "http://192.168.120.9:8000/test/result";
String serverNameSoftwareApiPushNoti = "http://192.168.120.9:8000/noti/api/upload/result";


// Cảm biến siêu âm
const int trigPin = D7;
const int echoPin = D8;
long duration;
int distance;


// servo
Servo servoMoNap;
Servo servoPhanLoai;  
const int servoPhanLoaiPin = D1; 
const int servoMoNapPin = D2;

String wasteType = "";


unsigned long previousMillis = 0; // Lưu trữ thời gian lần đo trước đó
const long interval = 500;  

void setup(){
  // put your setup code here, to run once:
  Serial.begin(115200);

  servoPhanLoai.attach(servoPhanLoaiPin,500,2400);
  servoPhanLoai.write(0);
  servoMoNap.attach(servoMoNapPin,500,2400);
  servoMoNap.write(0);
  delay(2000);
  servoMoNap.write(45);
  Serial.println("45");
  delay(2000);
  servoMoNap.write(90);
  Serial.println("90");
  delay(2000);
  servoMoNap.write(135);
  Serial.println("135");
  delay(2000);
  servoMoNap.write(180);
  Serial.println("180");
  delay(2000);

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
  Serial.println(WiFi.localIP());

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  server.on("/capture-complete", HTTP_GET, handleCaptureComplete);
  server.begin();

}

void loop() {
  server.handleClient();

  // Lấy thời gian hiện tại
  unsigned long currentMillis = millis();

  // Nếu khoảng thời gian đã trôi qua (500 ms), thực hiện đo khoảng cách
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Cập nhật thời gian lần đo trước đó

    // Đo khoảng cách từ cảm biến siêu âm
    long distance = getDistance();
    Serial.println(distance);

    // Kiểm tra xem có ai đến gần để gửi yêu cầu chụp ảnh
    if (distance > 0 && distance < 30) { // Nếu có người đến gần (khoảng cách < 30 cm và hợp lệ)
      sendCaptureRequest();
    }
  }

  // Nếu có kết quả phân loại, điều khiển động cơ
  if (wasteType != "") {
    controlServoMotor(wasteType); // Gọi hàm điều khiển động cơ
    wasteType = "";                 // Đặt lại loại rác để không quay lại sau
  }

}

long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;  // Tính toán khoảng cách (cm)
  return distance;
}

void sendCaptureRequest() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;  
    HTTPClient http;  // Sử dụng HTTPClient
    http.begin(client, esp32cam_url_capture);  // Gửi yêu cầu HTTP tới ESP32-CAM
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.println("Đã gửi yêu cầu chụp thành công.");
      String payload = http.getString();
      Serial.println("Phản hồi: " + payload);
    } else {
      Serial.printf("Lỗi khi gửi yêu cầu: %d\n", httpResponseCode);
    }
    http.end();
  }


}

String getResultFromServer() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;  
    HTTPClient http;
    http.begin(client, serverName_getResult);
  
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String payload  = http.getString();
      Serial.println("Result: " + payload);  // Hiển thị kết quả nhận diện

      // Phân tích chuỗi JSON
      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
        const char* result = doc["result"];  // Lấy giá trị "result"
        return String(result);
      } else {
        Serial.println("Lỗi phân tích JSON!");
      }
    } else {
      Serial.printf("Error on sending GET: %d\n", httpResponseCode);
    }
    http.end();
  }
  return "";
}

void controlServoMotor(String wasteType) {
  if (wasteType == "paper") {
    Serial.println(">>> Quay vao o 1");
    servoPhanLoai.write(22.5);
  } else if (wasteType == "plastic") {
    Serial.println(">>> Quay vao o 2");
    servoPhanLoai.write(67.5);
  } else if (wasteType == "glass") {
    Serial.println(">>> Quay vao o 3");
    servoPhanLoai.write(112.5);
  } else if (wasteType == "metal") {
    Serial.println(">>> Quay vao o 4");
    servoPhanLoai.write(157.5);
  }

  delay(1500);

  Serial.println(">>> Mo nap thung rac");
  servoMoNap.write(30);
  delay(1000);
  Serial.println(">>> Dong nap thung rac");
  servoMoNap.write(0);
  guiNoti(wasteType);
}

void handleCaptureComplete() {
  // Gọi API để lấy kết quả phân loại từ server
  String result = getResultFromServer();
  if (result != "") {
    Serial.println("Co ket qua");
    // wasteType = "";
    wasteType = result; // Cập nhật loại rác từ server
  } else {
    Serial.println("Không có kết quả từ server.");
  }

  server.send(200, "text/plain", "Result received and motor controlled");
  
}

void guiNoti(String rac) {
if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;  
    HTTPClient http;
    
    http.begin(client, serverNameSoftwareApiPushNoti);
    http.addHeader("Content-Type", "application/json");  // Thiết lập header cho yêu cầu JSON

    // Tạo chuỗi JSON
    String jsonPayload = "{\"result\":\""  + rac + "\",\"isFull\":false}";

    // Gửi yêu cầu POST với dữ liệu JSON
    int httpResponseCode = http.POST(jsonPayload);
    if (httpResponseCode > 0) {
      String payload = http.getString();
      Serial.println("Result: " + payload);  // Hiển thị kết quả nhận diện
    } else {
      Serial.printf("Error on sending POST: %d\n", httpResponseCode);
    }
    http.end();
  }
}