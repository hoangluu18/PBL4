#include <Wire.h>

// Định nghĩa các chân TRIG và ECHO cho 4 cảm biến
#define TRIG_PIN_1 2
#define ECHO_PIN_1 3
#define TRIG_PIN_2 4
#define ECHO_PIN_2 5
#define TRIG_PIN_3 6
#define ECHO_PIN_3 7
#define TRIG_PIN_4 8
#define ECHO_PIN_4 9

int n = 4;
int trigPins[] = {TRIG_PIN_1, TRIG_PIN_2, TRIG_PIN_3, TRIG_PIN_4};
int echoPins[] = {ECHO_PIN_1, ECHO_PIN_2, ECHO_PIN_3, ECHO_PIN_4};
float isFull[4] = {0.0, 0.0, 0.0, 0.0};

float distances[4] = {0.0, 0.0, 0.0, 0.0};

const float fullValue = 15;
const float DIST_MIN = 13.0;
const float DIST_MAX = 25.0;


int unstableCount[4] = {0, 0, 0, 0};
int cnt = 0;

void setup() {
    for (int i = 0; i < n; i++) {
        pinMode(trigPins[i], OUTPUT);
        pinMode(echoPins[i], INPUT);
    }

    Serial.begin(9600);
    Wire.begin(8);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
    Serial.println("Arduino Slave ready");
}

void loop() {
    Serial.println("");
    cnt++;
    for (int i = 0; i < 4; i++) {
        long duration;
        float distance;

        digitalWrite(trigPins[i], LOW);
        delayMicroseconds(2);
        digitalWrite(trigPins[i], HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPins[i], LOW);

        duration = pulseIn(echoPins[i], HIGH);
        distance = duration * 0.034 / 2;
        Serial.println("Cảm biến " + String(i+1) + ": "+ String(distance));
        if (distance < DIST_MIN || distance > DIST_MAX) {
            unstableCount[i]++;
        }
    }

    if (cnt == 10) {
      for(int i = 0; i < 4; i++) {
    Serial.println("isFull " + String(i+1) + ": " + String(isFull[i]));
  }
        for (int i = 0; i < n; i++) {
            if (unstableCount[i] * 2 > cnt) {
                Serial.println(String(i+1) + " : Đầy");
                isFull[i] = 1;
            } else {
                Serial.println(String(i+1) + " : Không Đầy");
                isFull[i] = 0;
            }
        }
        // Wire.write((byte*)isFull, sizeof(isFull));
        cnt = 0;
        memset(unstableCount, 0, sizeof(unstableCount));
    }
    delay(100);
} 

void requestEvent() {
  
    Wire.write((byte*)isFull, sizeof(isFull));

    for(int i = 0; i < 4; i++) {
      if(isFull[i] == 1) {
        Serial.println("Đầy");
      } else {
        Serial.println("Không đầy");
      }
    }

}

void receiveEvent(int howMany) {
    while (Wire.available()) {
        char c = Wire.read();
    }
}