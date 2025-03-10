#include <WiFi.h>                          // Library for WiFi connections
#include <HTTPClient.h>                    // Library for HTTP functions for client
#include <Wire.h>                          // Library for I2C communications
#include <Adafruit_GFX.h>                  // Library for display
#include <Adafruit_SSD1306.h>              // Library for OLED display
#include <Arduino.h>                       // Library for Ardunio functions

// Wi-Fi credentials
const char* ssid = "Engr Fahad";           // Replace with your Wi-Fi SSID
const char* password = "x73S3234";         // Replace with your Wi-Fi password

// Web server URL (Replace with your Flask server IP)
const char* serverURL = "https://marching-band-lps.onrender.com/update_positions/";  // Example: Flask server running on IP 192.168.100.3

HardwareSerial mySerial2(2);                // Use hardware serial 2 for communication with the UWB sensor

#define RESET 16                            // Set the RESET pin to pin 16
#define IO_RXD2 18                          // Set the RXD2 pin to pin 18
#define IO_TXD2 17                          // Set the TXD2 pin to pin 17
#define I2C_SDA 39                          // Set the SDA pin to pin 39
#define I2C_SCL 38                          // Set the SCL pin to pin 38

Adafruit_SSD1306 display(128, 64, &Wire, -1);  // OLED display initialization

String response = "";                       // Initialize response string
String rec_head = "AT+RANGE";               // String for AT commands

void setup() {                              // Function for setup
    pinMode(RESET, OUTPUT);                 // Set the RESET pin as output
    digitalWrite(RESET, HIGH);              // Set the RESET pin to HIGH to initialize UWB

    Serial.begin(115200);                   // Start the serial communication for debugging
    mySerial2.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2); // Initialize communication with UWB sensor
    Wire.begin(I2C_SDA, I2C_SCL);           // Initialize I2C for OLED display

    // Initialize OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {     // Check if OLED is connected
        Serial.println(F("SSD1306 allocation failed"));   // Print error message
        for (;;);  // Infinite loop if display initialization fails
    }
    display.clearDisplay();                 // Clear the display
    display.setTextSize(2);                 // Set text size
    display.setTextColor(SSD1306_WHITE);    // Set text color
    display.setCursor(0, 0);                // Start at the top-left corner
    display.println(F("UWB Ready"));        // Print message
    display.display();                      // Display message
    delay(2000);                            // Delay

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);                     // Connect to WiFi
    display.clearDisplay();                         // Clear the display
    display.setTextSize(1);                         // Set text size
    display.setCursor(0, 0);                        // Start at the top left corner
    display.println("Connecting to Wi-Fi...");      // Print message
    display.display();                              // Display message
    while (WiFi.status() != WL_CONNECTED) {         // Check if a WiFi connection was made
        delay(500);                                 // Delay
        Serial.print(".");                          // Print message
    }
    Serial.println("\nWi-Fi connected");            // Print message
    display.println("Wi-Fi Connected");             // Display message
    display.display();                              // Display
}

void loop() {                                       // Loop function
    // Read data from UWB sensor
    while (mySerial2.available() > 0) {             // While data is available
        char c = mySerial2.read();                  // Read the character
        if (c == '\r') continue;                    // Check if the character is a carriage return
        else if (c == '\n') {                       // Check if the character is new line or carriage return
            if (response.indexOf(rec_head) != -1) { // Check if the index of rec_head is not -1
                range_analy(response);              // Analyze range data when response header is found
            }
            response = "";                          // Reset response for the next data
        } else {
            response += c;                          // Accumulate the response from the sensor
        }
    }
}

// Analyze range data and send it to the web server
void range_analy(String data) {                   // Function to analyze data
    String id_str = data.substring(data.indexOf("tid:") + 4, data.indexOf(",mask:"));       // Create a string for ID
    String x_str = data.substring(data.indexOf("x:"), data.indexOf(",y:"));                 // Create a string for x-coordinates
    String y_str = data.substring(data.indexOf("y:"), data.indexOf(",rssi:"));              // Create a string for y-coordinates

    float x_coordinate = x_str.toFloat();                                                   // Make x-coordinates into a float
    float y_coordinate = y_str.toFloat();                                                   // Make y-coordinates into a float

    String json_str = "{\"tag_id\":\"" + id_str + "\",\"x_coordinate\":" + String(x_coordinate) + "\",\"y_coordinate\":" + String(y_coordinate) + "}";      // Combine all data to be sent out

    Serial.println(json_str);     // Print the JSON data to the Serial monitor
    sendDataToServer(json_str);   // Send data to the Flask server
}

// Send JSON data to the Flask server
void sendDataToServer(String jsonData) {                  // Function to send data to the server
    if (WiFi.status() == WL_CONNECTED) {                  // Check that WiFi is still connected
        HTTPClient http;                                  // Create a http
        http.begin(serverURL);                            // Connect to the server
        http.addHeader("Content-Type", "application/json");   // Add a header to the http indicating JSON

        int httpResponseCode = http.POST(jsonData);       // Send POST request with JSON data
        if (httpResponseCode > 0) {                       // Check that post sent
            String response = http.getString();           // Get the server's response
            Serial.println("Server Response: " + response);   // Print message
        } else {                                          // Otherwise
            Serial.println("Error sending data. HTTP Response Code: " + String(httpResponseCode));      // Print message
        }
        http.end();                                       // End the HTTP request
    } else {
        Serial.println("Wi-Fi disconnected");             // Print message
    }
}