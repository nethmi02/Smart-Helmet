#include <Ethernet.h>
#include <SPI.h>
#include <Wire.h>
#include <MPU6050.h>
#include <TinyGPS++.h>

// Ethernet settings
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // Unique MAC address
IPAddress ip(192, 168, 1, 177); // Static IP (adjust to your network)
const char* serverUrl = "yourserver.com"; // Replace with your server domain
const int serverPort = 80; // HTTP port

// GPS setup (using Serial1)
TinyGPSPlus gps;
#define GPS_RX 19 // Mega Serial1 RX (connect to GPS TX)
#define GPS_TX 18 // Mega Serial1 TX (connect to GPS RX)

// MPU6050 setup
MPU6050 mpu;
int16_t ax, ay, az, gx, gy, gz; // Changed to int16_t for raw data

// Sampling rates (in milliseconds)
int gpsSendInterval = 20000;    // 20 seconds
int collisionCheckInterval = 60000; // 1 minute

// Ethernet client
EthernetClient client;

void sendGPSData() {
    if (gps.location.isValid()) {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();

        if (client.connect(serverUrl, serverPort)) {
            String payload = "{\"type\":\"gps\",\"latitude\":" + String(latitude, 6) +
                             ",\"longitude\":" + String(longitude, 6) + "}";
            client.println("POST /data HTTP/1.1");
            client.println("Host: " + String(serverUrl));
            client.println("Content-Type: application/json");
            client.print("Content-Length: ");
            client.println(payload.length());
            client.println();
            client.println(payload);

            delay(100);
            while (client.available()) {
                Serial.write(client.read());
            }
            client.stop();
            Serial.println("GPS data sent: " + payload);
        } else {
            Serial.println("GPS connection failed");
        }
    } else {
        Serial.println("GPS data not valid yet");
    }
}

void sendMPUData() {
    float avgAx = 0, avgAy = 0, avgAz = 0, avgGx = 0, avgGy = 0, avgGz = 0;
    int samples = 10;

    for (int i = 0; i < samples; i++) {
        mpu.getAcceleration(&ax, &ay, &az); // Now matches int16_t*
        mpu.getRotation(&gx, &gy, &gz);     // Now matches int16_t*
        
        // Convert raw values to physical units during averaging
        avgAx += ax / 16384.0; // ±2g range (accel sensitivity)
        avgAy += ay / 16384.0;
        avgAz += az / 16384.0;
        avgGx += gx / 131.0;   // ±250°/s range (gyro sensitivity)
        avgGy += gy / 131.0;
        avgGz += gz / 131.0;

        delay(10);
    }

    avgAx /= samples; avgAy /= samples; avgAz /= samples;
    avgGx /= samples; avgGy /= samples; avgGz /= samples;

    if (client.connect(serverUrl, serverPort)) {
        String payload = "{\"type\":\"mpu6050\",\"ax\":" + String(avgAx, 2) +
                         ",\"ay\":" + String(avgAy, 2) + ",\"az\":" + String(avgAz, 2) +
                         ",\"gx\":" + String(avgGx, 2) + ",\"gy\":" + String(avgGy, 2) +
                         ",\"gz\":" + String(avgGz, 2) + "}";
        client.println("POST /data HTTP/1.1");
        client.println("Host: " + String(serverUrl));
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(payload.length());
        client.println();
        client.println(payload);

        delay(100);
        while (client.available()) {
            Serial.write(client.read());
        }
        client.stop();
        Serial.println("MPU data sent: " + payload);
    } else {
        Serial.println("MPU connection failed");
    }
}

void setup() {
    Serial.begin(115200);    // For debugging
    Serial1.begin(9600);     // GPS on Serial1 (pins 18, 19)

    // Ethernet initialization
    if (Ethernet.begin(mac) == 0) {
        Serial.println("Failed to configure Ethernet using DHCP");
        Ethernet.begin(mac, ip); // Fallback to static IP
    }
    delay(1000);
    Serial.print("Ethernet IP: ");
    Serial.println(Ethernet.localIP());

    // MPU6050 setup
    Wire.begin();
    mpu.initialize();
    if (mpu.testConnection()) {
        Serial.println("MPU6050 connected");
    } else {
        Serial.println("MPU6050 connection failed");
    }
}

void loop() {
    static unsigned long lastGPSSendTime = 0;
    static unsigned long lastMPUCheckTime = 0;

    // Read GPS data
    while (Serial1.available() > 0) {
        gps.encode(Serial1.read());
    }

    // Send GPS data
    if (millis() - lastGPSSendTime >= gpsSendInterval) {
        sendGPSData();
        lastGPSSendTime = millis();
    }

    // Send MPU6050 data
    if (millis() - lastMPUCheckTime >= collisionCheckInterval) {
        sendMPUData();
        lastMPUCheckTime = millis();
    }
}
