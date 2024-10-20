#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h> 
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
// #include <Stepper.h>
#include <AccelStepper.h>

const char *ssid = "Viet";
const char *password = "0902042212";

ESP8266WebServer server(80);

// url
String esp32cam_url_capture = "http://192.168.1.6/capture";
String serverName_getResult = "http://192.168.1.2:8000/result";


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
const int stepsPerRevolution  = 2048;
// Stepper stepper= Stepper(stepsPerRevolution, IN1, IN3, IN2, IN4);
AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);
String wasteType = "";

void setup(){
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Khoi dong lai setup");
  // speed motor
  // stepper.setSpeed(15);
  stepper.setMaxSpeed(500);
  stepper.setAcceleration(100);
  stepper.moveTo(stepsPerRevolution);
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
  // Nếu đã nhận loại rác, điều khiển động cơ

  server.handleClient();
  // Đo khoảng cách từ cảm biến siêu âm
  long distance = getDistance();
  Serial.println(distance);
  if (distance < 30) { // Nếu có người đến gần (khoảng cách < 30 cm)
    sendCaptureRequest();
    Serial.println("Đã gửi yêu cầu chụp");
  }
  if (wasteType != "") {
    controlStepperMotor(wasteType); // Gọi hàm điều khiển động cơ
    wasteType = ""; // Đặt lại loại rác để không quay lại sau
  }
  stepper.run();
   // Gọi thường xuyên để động cơ hoạt động
  delay(1000);  // Kiểm tra lại sau 1 giây
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
  String res;
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;  
    HTTPClient http;
    http.begin(client, serverName_getResult);
  
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
       res = http.getString();
      Serial.println("Result: " + res);  // Hiển thị kết quả nhận diện
      // Xử lý kết quả để điều khiển động cơ bước
    } else {
      Serial.printf("Error on sending GET: %d\n", httpResponseCode);
    }
    http.end();
  }
  return res;
}

void rotateStepperToPosition(int targetAngle) {
  int steps = (targetAngle / 360.0) * stepsPerRevolution;
  // stepper.step(steps);
  // delay(500); // Đợi động cơ ổn định
  stepper.moveTo(steps); // Set target position
  stepper.runToPosition(); // Move stepper to target
}
void controlStepperMotor(String wasteType){
  if (wasteType == "glass") {
    rotateStepperToPosition(0);  // Rác hữu cơ
  } else if (wasteType == "metal") {
    rotateStepperToPosition(90);  // Rác tái chế
  } else if (wasteType == "paper") {
    rotateStepperToPosition(180);  // Rác không tái chế
  } else if (wasteType == "plastic") {
    rotateStepperToPosition(270);  // Rác điện tử
  }

  // Sau khi điều khiển xong, có thể chờ động cơ di chuyển
  while (stepper.distanceToGo() != 0) {
    stepper.run(); // Chạy cho đến khi động cơ đến vị trí
  }
}

void handleCaptureComplete() {
  // Gọi API để lấy kết quả phân loại từ server
  // wasteType = getResultFromServer();
    String result = getResultFromServer();
  if (result != "") {
    wasteType = result; // Cập nhật loại rác từ server
  } else {
    Serial.println("Không có kết quả từ server.");
  }
  // Sau khi lấy kết quả, điều khiển động cơ bước
  // controlStepperMotor(res);

  server.send(200, "text/plain", "Result received and motor controlled");
  
}