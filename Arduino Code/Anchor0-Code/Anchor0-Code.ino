#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Arduino.h>

// Wi-Fi credentials
// ESP32 Board MAC Address: f8:b3:b7:4f:dc:6c


//Alex Apartment WiFi
// const char* ssid = "WhiteSky-Hudson";           // Replace with your Wi-Fi SSID
// const char* password = "d3xey8kz";      // Replace with your Wi-Fi password

//TAMU WiFi
//const char* ssid = "TAMU_IoT";           // Replace with your Wi-Fi SSID

//Alex Phone WiFi
// const char* ssid = "AlexPhone";           // Replace with your Wi-Fi SSID
// const char* password = "1Alexander";      // Replace with your Wi-Fi password

//Alex House
const char* ssid = "FBI Van";           // Replace with your Wi-Fi SSID
const char* password = "steve1107";      // Replace with your Wi-Fi password

// Web server URL (Replace with your Flask server IP)
const char* serverURL = "https://marching-band-lps.onrender.com/BandField/update_positions/";  // Example: Flask server running on IP 192.168.100.3

HardwareSerial mySerial2(2);  // Use hardware serial 2 for communication with the UWB sensor

#define RESET 13
#define IO_RXD2 2
#define IO_TXD2 15

#define UWB_INDEX 0

#define ANCHOR

#define UWB_TAG_COUNT 10

#define SERIAL_LOG Serial
#define SERIAL_AT mySerial2

void setup() {
    pinMode(RESET, OUTPUT);
    digitalWrite(RESET, HIGH);

    SERIAL_LOG.begin(115200);

    SERIAL_LOG.println("Hello! Marching Band LPS Test Commencing");
    SERIAL_AT.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);
    delay(3000);
    SERIAL_AT.println("AT");

    sendData("AT?", 2000, 1);
    sendData("AT+RESTORE", 5000, 1);
    sendData(config_cmd(), 2000, 1);
    sendData(cap_cmd(), 2000, 1);
    sendData("AT+SETRPT=1", 2000, 1);
    sendData("AT+SAVE", 2000, 1);
    sendData("AT+RESTART", 2000, 1);
    delay(3000);  // Wait for UWB modulo9=-999e to finish booting

    // //Connect to Wi-Fi
    // WiFi.begin(ssid, password);
    // int i = 0;
    // Serial.println("\nConnecting to Wi-Fi");
    // //checks fo wifi status, but will continue if no wifi is detected
    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(1000);
    //     Serial.print(".");
    //     i += 1;
    //     if (i > 10){
    //       Serial.println("\nNo Wifi detected, continuing in offline mode");
    //       break;
    //     }
    // }
    // if (WiFi.status() == WL_CONNECTED){
    //   Serial.println("\nWi-Fi connected");
    // }
    
}
long int runtime = 0;
String response = "";
String rec_head = "AT+RANGE";

void loop(){
    // put your main code here, to run repeatedly:
    while (SERIAL_LOG.available() > 0){
        SERIAL_AT.write(SERIAL_LOG.read());
        yield();
    }
    while (SERIAL_AT.available() > 0){
        char c = SERIAL_AT.read();
        if (c == '\r')
            continue;
        else if (c == '\n' || c == '\r'){
            range_analy(response);
            response = "";
        }
        else
            response += c;
}
}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    // command = command + "\r\n";

    SERIAL_LOG.println(command);
    SERIAL_AT.println(command); // send the read character to the SERIAL_LOG

    long int time = millis();

    while ((time + timeout) > millis())
    {
        while (SERIAL_AT.available())
        {

            // The esp has data so display its output to the serial window
            char c = SERIAL_AT.read(); // read the next character.
            response += c;
        }
    }

    if (debug)
    {
        SERIAL_LOG.println(response);
    }

    return response;                
}

String config_cmd()
{
    String temp = "AT+SETCFG=";

    // Set device id
    temp = temp + UWB_INDEX;
    
    // Set device role
    //x2:Device Role(0:Tag / 1:Anchor)
    temp = temp + ",1";


    // Set frequence 850k or 6.8M

    temp = temp + ",1";

    // Set range filter
    temp = temp + ",1";

    return temp;
}

String cap_cmd()
{
    String temp = "AT+SETCAP=";

    // Set Tag capacity
    temp = temp + UWB_TAG_COUNT;

    //  Time of a single time slot  6.5M : 10MS  850K ： 15MS
    temp = temp + ",10";
    
    //X3:extMode, whether to increase the passthrough command when transmitting
    //(0: normal packet when communicating, 1: extended packet when communicating)
    temp = temp + ",1";

    return temp;
}

// Analyze range data and send it to the web server
void range_analy(String data) {
    //getting range data
    String id_str = data.substring(data.indexOf("tid:") + 4, data.indexOf(",mask:"));
    String range_data = data.substring(data.indexOf("range:"), data.indexOf(",rssi:"));
 
    int range_list[8];
    int count = sscanf(range_data.c_str(), "range:(%d,%d,%d,%d,%d,%d,%d,%d)",
                       &range_list[0], &range_list[1], &range_list[2], &range_list[3],
                       &range_list[4], &range_list[5], &range_list[6], &range_list[7]);

    // if (count != 8) {
    //     Serial.println("RANGE ANALY ERROR");
    //     return;
    // }
 
    String json_str = "{\"id\":" + id_str + ",\"range\":[";
    for (int i = 0; i < 8; i++) {
        json_str += String(range_list[i]);
        if (i < 7) json_str += ",";
    }
    json_str += "]}";
 
    Serial.println(json_str);
    ConvRangetoPos(id_str, range_list);
}

void ConvRangetoPos(String id, int range_list[8]){    
  //definine anchor coords
    float x1 = 0, y1 = 0; //anchor 0
    float x2 = 975.36, y2 = 0; //anchor 1
    float x3 = 487.68, y3 = 1097.28; //anchor 2

    x1 += 32;
    //y1 += 32;
    x2 += 32;
    //y2 += 32;
    x3 += 32;
    y3 += 32;

    // extract range values
    float d1 = range_list[0];
    float d2 = range_list[1];
    float d3 = range_list[2];
    //Serial.println("I am computing");

    // Trilateration calculations
    float A = 2 * (x2 - x1);
    float B = 2 * (y2 - y1);
    float C = pow(d1, 2) - pow(d2, 2) - pow(x1, 2) + pow(x2, 2) - pow(y1, 2) + pow(y2, 2);

    float D = 2 * (x3 - x1);
    float E = 2 * (y3 - y1);
    float F = pow(d1, 2) - pow(d3, 2) - pow(x1, 2) + pow(x3, 2) - pow(y1, 2) + pow(y3, 2);

    // Calculate denominator to check validity
    float denominator = A * E - B * D;

    // Check for invalid solutions (denominator close to zero -> no solution or parallel)
    if (abs(denominator) < 0.00001) {
    Serial.println("Error: Invalid intersection or no solution.");
    return;  // Exit if no valid intersection
    }

    // Solve for x and y using Cramer's rule
    float x_coord = (C * E - F * B) / denominator;
    float y_coord = (A * F - C * D) / denominator;
    id = "James";

    //String test_str = "{\"tag_id\":\"" + id + "\",\"x_coordinate\":" + String(x_coord) + ",\"y_coordinate\":" + String(y_coord) + "}";
    // Serial.println(test_str);  // Print the JSON data to the Serial monitor\

    //testing if x or y is negative
    if (x_coord < 0){
      Serial.println("Negative X coordinate");
      return;
    }
    if (y_coord < 0){
      Serial.println("Negative Y coordinate");
      return;
    }

    // testing if x or y is too large
    if (x_coord > 20000 ){
      Serial.println("Bad data: y too big");
      return;
    }
    if (y_coord > 20000){
      Serial.println("Bad data: y too big");
      return;
    }

    String json_str = "{\"tag_id\":\"" + id + "\",\"x_coordinate\":" + String(x_coord) + ",\"y_coordinate\":" + String(y_coord) + "}";
    Serial.println(json_str);  // Print the JSON data to the Serial monitor
    sendDataToServer(json_str);  // Send data to the Flask server}
}

// Send JSON data to the Flask server
void sendDataToServer(String jsonData) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverURL);  // Connect to the Flask server
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Referer","https://marching-band-lps.onrender.com");

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
