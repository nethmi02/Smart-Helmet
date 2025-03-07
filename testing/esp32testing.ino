#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <MPU6050.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <ESPAsyncWebServer.h>

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Server endpoint (replace with your actual server URL)
const char* serverUrl = "http://yourserver.com/data";

// GPS setup
HardwareSerial gpsSerial(1);
TinyGPSPlus gps;

// MPU6050 setup
MPU6050 mpu;
float ax, ay, az, gx, gy, gz;

// Sampling rates (default values)
int gpsSendInterval = 20000; // 20 seconds
int collisionCheckInterval = 60000; // 1 minute

// Web server to change sampling rate
AsyncWebServer server(80);

// Function to send GPS data
void sendGPSData() {
    if (gps.location.isValid()) {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();

        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        String payload = "{\"type\":\"gps\",\"latitude\":" + String(latitude) +
                         ",\"longitude\":" + String(longitude) + "}";

        int httpResponseCode = http.POST(payload);
        http.end();
    }
}

// Function to check collision and send MPU6050 data
void sendMPUData() {
    float avgAx = 0, avgAy = 0, avgAz = 0, avgGx = 0, avgGy = 0, avgGz = 0;
    int samples = 10; // Number of samples for averaging

    for (int i = 0; i < samples; i++) {
        mpu.getAcceleration(&ax, &ay, &az);
        mpu.getRotation(&gx, &gy, &gz);
        
        avgAx += ax; avgAy += ay; avgAz += az;
        avgGx += gx; avgGy += gy; avgGz += gz;

        delay(10);
    }

    avgAx /= samples; avgAy /= samples; avgAz /= samples;
    avgGx /= samples; avgGy /= samples; avgGz /= samples;

    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"type\":\"mpu6050\",\"ax\":" + String(avgAx) +
                     ",\"ay\":" + String(avgAy) + ",\"az\":" + String(avgAz) +
                     ",\"gx\":" + String(avgGx) + ",\"gy\":" + String(avgGy) +
                     ",\"gz\":" + String(avgGz) + "}";

    int httpResponseCode = http.POST(payload);
    http.end();
}

// Function to handle sampling rate change via API
void handleSamplingRateChange(AsyncWebServerRequest *request) {
    if (request->hasParam("gps") && request->hasParam("mpu")) {
        gpsSendInterval = request->getParam("gps")->value().toInt();
        collisionCheckInterval = request->getParam("mpu")->value().toInt();
        request->send(200, "text/plain", "Sampling rates updated");
    } else {
        request->send(400, "text/plain", "Missing parameters");
    }
}

void setup() {
    Serial.begin(115200);
    
    // WiFi Connection
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // GPS setup
    gpsSerial.begin(9600, SERIAL_8N1, 16, 17); // TX=16, RX=17

    // MPU6050 setup
    Wire.begin();
    mpu.initialize();

    // API to change sampling rates
    server.on("/set_sampling", HTTP_GET, handleSamplingRateChange);
    server.begin();
}

void loop() {
    static unsigned long lastGPSSendTime = 0;
    static unsigned long lastMPUCheckTime = 0;

    // Read GPS data continuously
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }

    // Send GPS data every gpsSendInterval
    if (millis() - lastGPSSendTime > gpsSendInterval) {
        sendGPSData();
        lastGPSSendTime = millis();
    }

    // Send MPU6050 data every collisionCheckInterval
    if (millis() - lastMPUCheckTime > collisionCheckInterval) {
        sendMPUData();
        lastMPUCheckTime = millis();
    }
}
