#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

// Wi-Fi credentials
const char* ssid = "Engr Fahad";           // Replace with your Wi-Fi SSID
const char* password = "x73S3234";      // Replace with your Wi-Fi password

// Web server URL (Replace with your Flask server IP)
const char* serverURL = "https://marching-band-lps.onrender.com";  // Example: Flask server running on IP 192.168.100.3

HardwareSerial mySerial2(2);  // Use hardware serial 2 for communication with the UWB sensor

#define RESET 16
#define IO_RXD2 18
#define IO_TXD2 17
#define I2C_SDA 39
#define I2C_SCL 38

Adafruit_SSD1306 display(128, 64, &Wire, -1);  // OLED display initialization

String response = "";
String rec_head = "AT+RANGE";

void setup() {
    pinMode(RESET, OUTPUT);
    digitalWrite(RESET, HIGH);

    Serial.begin(115200);          // Start the serial communication for debugging
    mySerial2.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2); // Initialize communication with UWB sensor

    Wire.begin(I2C_SDA, I2C_SCL);  // Initialize I2C for OLED display

    // Initialize OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);  // Infinite loop if display initialization fails
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("UWB Ready"));
    display.display();
    delay(2000);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Connecting to Wi-Fi...");
    display.display();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connected");
    display.println("Wi-Fi Connected");
    display.display();
}

void loop() {
    // Read data from UWB sensor
    while (mySerial2.available() > 0) {
        char c = mySerial2.read();
        if (c == '\r') continue;
        else if (c == '\n') {
            if (response.indexOf(rec_head) != -1) {
                range_analy(response);  // Analyze range data when response header is found
            }
            response = "";  // Reset response for the next data
        } else {
            response += c;  // Accumulate the response from the sensor
        }
    }
}

// Analyze range data and send it to the web server
void range_analy(String data) {
    String id_str = data.substring(data.indexOf("tid:") + 4, data.indexOf(",mask:"));
    String range_str = data.substring(data.indexOf("range:"), data.indexOf(",rssi:"));

    int range_list[8];
    int count = sscanf(range_str.c_str(), "range:(%d,%d,%d,%d,%d,%d,%d,%d)",
                       &range_list[0], &range_list[1], &range_list[2], &range_list[3],
                       &range_list[4], &range_list[5], &range_list[6], &range_list[7]);

    if (count != 8) {
        Serial.println("RANGE ANALY ERROR");
        return;
    }

    String json_str = "{\"id\":" + id_str + ",\"range\":[";
    for (int i = 0; i < 8; i++) {
        json_str += String(range_list[i]);
        if (i < 7) json_str += ",";
    }
    json_str += "]}";

    Serial.println(json_str);  // Print the JSON data to the Serial monitor
    sendDataToServer(json_str);  // Send data to the Flask server
}

// Send JSON data to the Flask server
void sendDataToServer(String jsonData) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverURL);  // Connect to the Flask server
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.POST(jsonData);  // Send POST request with JSON data
        if (httpResponseCode > 0) {
            String response = http.getString();  // Get the server's response
            Serial.println("Server Response: " + response);
        } else {
            Serial.println("Error sending data. HTTP Response Code: " + String(httpResponseCode));
        }
        http.end();  // End the HTTP request
    } else {
        Serial.println("Wi-Fi disconnected");
    }
}