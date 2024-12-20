#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h> 
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <Stepper.h>
#include <AccelStepper.h>
#include <ArduinoJson.h>
#include <Servo.h>

// const char *ssid = "Viet";
// const char *password = "0902042212";
const char *ssid = "zzz";
const char *password = "123456789";
ESP8266WebServer server(80);

// url
String esp32cam_url_capture = "http://192.168.22.244/capture";
String serverName_getResult = "http://192.168.22.22:8000/result";


// Cảm biến siêu âm
const int trigPin = D7;
const int echoPin = D8;
long duration;
int distance;

// step motor
// ULN2003 Motor Driver Pins
#define IN1 5 //d1
#define IN2 4 //d2
#define IN3 14 //d5
#define IN4 12 //d6
// servo
Servo servoMoNap;
Servo servoPhanLoai;  
const int servoMoNapPin = D4;
const int servoPhanLoaiPin = ; 
// const int stepsPerRevolution  = 2048;
const int stepsPerRevolution  = 4096;
AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);
String wasteType = "";
int currentPosition = 0;

unsigned long previousMillis = 0; // Lưu trữ thời gian lần đo trước đó
const long interval = 500;  

void setup(){
  // put your setup code here, to run once:
  Serial.begin(115200);
  // speed motor
  stepper.setMaxSpeed(500);
  stepper.setAcceleration(100);
  myServo.attach(servoPin);
  // myServo.write(0);

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
      Serial.println("Đã gửi yêu cầu chụp");
    }
  }

  // Nếu có kết quả phân loại, điều khiển động cơ
  if (wasteType != "") {
    controlStepperMotor(wasteType); // Gọi hàm điều khiển động cơ
    // Serial.println("mở nắp thùng");
    wasteType = "";                 // Đặt lại loại rác để không quay lại sau
  }

  stepper.run(); // Gọi thường xuyên để động cơ hoạt động

  // dalay(500);
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

void controlStepperMotor(String wasteType) {
  int targetPosition = 0; 
  Serial.print(wasteType);
  if (wasteType == "paper") {
    targetPosition = 0;  // Compartment 1 for paper
  } else if (wasteType == "plastic") {
    targetPosition = 1; // Compartment 2 for plastic
  } else if (wasteType == "glass") {
    targetPosition = 2; // Compartment 3 for glass
  } else if (wasteType == "metal") {
    targetPosition = 3; // Compartment 4 for metal
  }
  if (targetPosition != currentPosition) {
    int stepsToMove = (targetPosition - currentPosition) * (4096 / 4);
    Serial.print(">>> check: ");
    Serial.println(stepsToMove);
    Serial.println(stepper.currentPosition());

    // stepper.moveTo(stepper.currentPosition() + stepsToMove);
    stepper.moveTo(stepsToMove);
    Serial.print(">>> check cc: ");
    Serial.println(stepper.currentPosition());
    stepper.runToPosition();
    stepper.setCurrentPosition(0);
    currentPosition = targetPosition; // Cập nhật lại vị trí hiện tại sau khi di chuyển
  }

  while (stepper.distanceToGo() != 0) {
    stepper.run(); // Chạy cho đến khi động cơ đến vị trí
    yield(); // Thêm hàm yield() để cho phép hệ thống xử lý tác vụ nền
  }
  if (stepper.distanceToGo() == 0) {
    Serial.println("Đã quay đến đúng vị trí, mở nắp thùng rác.");
    myServo.write(180);  // Mở nắp thùng
    delay(2000);               // Giữ nắp mở trong 2 giây (tuỳ chỉnh)
    myServo.write(0); // Đóng nắp thùng sau 2 giây
    delay(2000); 
    Serial.println("Đóng nắp thùng rác.");
  }
  
}

void constrolServor(String wasteType) {

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