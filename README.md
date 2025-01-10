
# PBL4 - Smart Trash Bin Project

## Overview

This project is a smart trash bin system that uses various technologies to detect and classify waste. The system consists of multiple components, including an ESP8266 microcontroller, an ESP32-CAM module, a mobile application, and a backend server. The system aims to automate waste classification and provide real-time updates on the status of the trash bins.

### Components

1.  **ESP8266**: Controls the servo motors and communicates with the ESP32-CAM and the backend server.
2.  **ESP32-CAM**: Captures images of the waste and sends them to the backend server for classification.
3.  **Mobile App**: A Flutter-based application that provides a user interface for monitoring the status of the trash bins.
4.  **Backend Server**: A FastAPI-based server that handles image classification and stores data in Firebase.
5.  **Model**: Transfer Learning with the InceptionV3 Model
## Setup Instructions

### ESP8266

1.  **Libraries Required**:
    
    -   ESP8266WiFi
    -   ESP8266WebServer
    -   WiFiClient
    -   ESP8266HTTPClient
    -   ArduinoJson
    -   Servo
    -   Wire
2.  **Configuration**:
    
    -   Set the WiFi credentials in the  servo_control.ino  file.
    -   Define the server URLs for communication.
3.  **Upload Code**:
    
    -   Use the Arduino IDE to upload the code in the  servo_control.ino  file.

### ESP32-CAM

1.  **Libraries Required**:
    
    -   esp_camera
    -   WiFi
    -   HTTPClient
    -   WebServer
    -   ArduinoJson
2.  **Configuration**:
    
    -   Set the WiFi credentials in the  esp32Cam.ino  file.
    -   Define the server URLs for communication.
3.  **Upload Code**:
    
    -   Use the Arduino IDE to upload the code in the  esp32Cam.ino  file.

### Mobile App

1.  **Requirements**:
    
    -   Flutter SDK
    -   Dart
2.  **Setup**:
    
    -   Navigate to the  `Mobile_App`  directory.
    -   Run  flutter pub get  to install dependencies.
3.  **Build and Run**:
    
    -   Use  `flutter run`  to build and run the application on a connected device or emulator.

### Backend Server

1.  **Requirements**:
    
    -   Python 3.8+
    -   FastAPI
    -   Firebase Admin SDK
    -   Cloudinary
2.  **Setup**:
    
    -   Navigate to the  `PBL4_Server_Backend_API`  directory.
    -   Install dependencies using  `pip install -r requirements.txt`.
3.  **Configuration**:
    
    -   Set up Firebase credentials and Cloudinary configuration in the  ResultSender.py  file.
4.  **Run Server**:
    
    -   Use  `uvicorn main:app --reload`  to start the server.

## Usage

1.  **ESP8266**:
    
    -   Controls the servo motors to open and close the trash bin lids.
    -   Communicates with the ESP32-CAM to capture images and send them to the backend server.
2.  **ESP32-CAM**:
    
    -   Captures images of the waste and sends them to the backend server for classification.
    -   Notifies the ESP8266 of the classification result.
3.  **Mobile App**:
    
    -   Provides a user interface for monitoring the status of the trash bins.
    -   Displays real-time updates on the fill level and type of waste in each bin.
4.  **Backend Server**:
    
    -   Handles image classification using a pre-trained model.
    -   Stores data in Firebase and provides APIs for communication with the ESP8266 and ESP32-CAM.
    
## System Diagram
1.  **Overall System Diagram**:

![Overall System Diagram](https://res.cloudinary.com/daku3kfyd/image/upload/v1736322552/Picture2_uw0bjv.png)

2.  **Circuit Installation Diagram**:

![Overall System Diagram](https://res.cloudinary.com/daku3kfyd/image/upload/v1736322553/Picture3_mfwloe.png)

3.  **Waste Classification System Model**:

![Overall System Diagram](https://res.cloudinary.com/daku3kfyd/image/upload/v1736322552/Picture4_vffb3a.jpg)

## Mobile

1.  **Main Screen**:

![Overall System Diagram](https://res.cloudinary.com/daku3kfyd/image/upload/v1736322552/Picture5_tfv3xc.jpg)

2.  **Classification History**:

![Overall System Diagram](https://res.cloudinary.com/daku3kfyd/image/upload/v1736322552/Picture6_lgajct.jpg)

3.  **Waste Classification Details**:

![Overall System Diagram](https://res.cloudinary.com/daku3kfyd/image/upload/v1736322552/Picture7_dxavvg.jpg)
## AI
AI Model for Waste Classification in the **model** Folder

## License

This project is licensed under the MIT License.

