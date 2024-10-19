#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h> 
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
const char *ssid = "Viet";
const char *password = "0902042212";

ESP8266WebServer server(80);

// url
String esp32cam_url_capture = "http://192.168.1.5/capture";

// Cảm biến siêu âm
const int trigPin = D1;
const int echoPin = D2;
long duration;
int distance;


void setup(){
  // put your setup code here, to run once:
  Serial.begin(115200);
  
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
  
  
}

void loop() {
  // Đo khoảng cách từ cảm biến siêu âm
  long distance = getDistance();
  Serial.println(distance);
  if (distance < 30) { // Nếu có người đến gần (khoảng cách < 30 cm)
    // WiFiClient client;
    // if (client.connect(esp32cam_url_capture, 80)) {
    //   client.print("GET /capture HTTP/1.1\r\nHost: ");
    //   client.print(esp32cam_url_capture);
    //   client.print("\r\nConnection: close\r\n\r\n");
    //   Serial.println("Da goi");
    //   delay(500);
    //   client.stop();
    // }
    sendCaptureRequest();
    Serial.println("Da gui");
  }
  
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
      Serial.println("Capture request sent successfully.");
      String payload = http.getString();
      Serial.println("Response: " + payload);
    } else {
      Serial.printf("Error sending request: %d\n", httpResponseCode);
    }
    http.end();
  }
}