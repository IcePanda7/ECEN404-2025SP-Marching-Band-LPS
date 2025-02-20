
// Defines
//#define IS_TAG    // Define a tag
#define IS_ANCHOR   // Define a anchor

// Includes
#include <SPI.h>                // SPI interface required communications between ESP32 and DW1000
#include <DW1000Ranging.h>      // DW-1000 specific library                   xxxxxx

#ifdef IS_ANCHOR                 // Check if device is an anchor
  #include <WiFi.h>              // WiFi library
  #include <WiFiUdp.h>           // WiFi UDP library
  #include  "link.h"             // Linked list with data                      **********
  #include <HTTPClient.h>        // Send HTTP Post requests library
  #include "FS.h"                // Library to handle file systems (read/write)
  #include "SPIFFS.h"            // Library to store files in ESP32 flash memory
#endif

// Every UWB device must have a unique EUI
#define DEVICE_ADDRESS "02:00:00:00:00:00:00:02"        // Unique address for device X


#ifdef IS_ANCHOR                                        // Check if device is an anchor
  const char *ssid = "Insert SSID here";                // Get the WiFi SSID
  const char *password = "SSID password here";          // Get the WiFi password

  const char* server = "https://marching-band-lps.onrender.com";                // Sever backend URL      XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

  // const char *host = "192.168.0.155";
  // uint_t portNum = 50000;
#endif


#ifdef IS_ANCHOR                                        // Check if device is an anchor
  WiFiUDP udp;                                          // Create UDP object
  struct MyLink *uwb_data;                              // Create a pointer to the linked list to store anchor data
  unsigned long lastUpdateTime = 0;                     // Track when data was sent
  unsigned int updateInterval = 200;                    // Send data every 200 miliseconds
#endif

char shortAddress[6];                                   // Short address for devices on network

void setup(){                                                 // Function for setup
  Serial.begin(115200);                                       // Start serial communication
  Serial.println(__FILE__ __DATE__);                          // Display the file name and date

  SPI.begin(18, 19, 23);                                      // Initialise SPI interface on specified SCK, MISO, MOSI pins
  DW1000Ranging.initCommunication(27, 4, 34);                 // Start up DW1000 chip on specified RESET, CS, and IRQ pins
  DW1000Ranging.attachNewRange(newRange);                     // Callback handler when the distance to a known tag changes
  DW1000Ranging.attachNewDevice(newDevice);                   // Callback handler when a new device is found on the network
  DW1000Ranging.attachInactiveDevice(inactiveDevice);         // Callback handler when a device is removed on the network


  #ifdef IS_TAG                                               // Check if device is a tag
    DW1000Ranging.startAsTag(DEVICE_ADDRESS, DW1000.MODE_LONGDATA_RANGE_ACCURACY, false);       // 110kb/s data rate, 64MHz PRF and long preambles
    display.printFixed(0, 0, "TAG", STYLE_NORMAL);            // Display "TAG"

  #elif defined(IS_ANCHOR)
    DW1000Ranging.startAsAnchor(DEVICE_ADDRESS, DW1000.MODE_LONGDATA_RANGE_ACCURACY, false);         // Must match the same mode for anchor
    display.printFixed(0, 0, "ANCHOR", STYLE_NORMAL);         // Display "ANCHOR"
    //uwb_data = init_link();                                 // Initialise array to keep track of links to all anchors
    
    WiFi.mode(WIFI_STA);                                      // Set WiFi mode
    WiFi.setSleep(false);                                     // Disable WiFi sleep
    WiFi.begin(ssid, password);                               // Connect to the WiFi network
    while (WiFi.status() != WL_CONNECTED){                    // While the WiFi hasn't connected
      delay(500);                                             // Delay for half a second
      Serial.print(".");                                      // Display a dot
    }

    Serial.println(F("Connected"));                           // Display that the WiFi successfully connected
    Serial.print(F("IP Address:"));                           // Display
    Serial.println(WiFi.localIP());                           // Display the local IP
    delay(500);                                               // Delay for half a second

    if(!SPIFFS.begin(true)){                                  // Check if the SPIFFS file system is operating
      Serial.println("SPIFFS error");                         // Display error message
      return;
    }
  #endif

  Serial.println(DEVICE_ADDRESS);                             // Display device address
  display.printFixed(0, 8, DEVICE_ADDRESS, STYLE_NORMAL);     // Display device address

  byte* currentShortAddress = DW1000Ranging.getCurrentShortAddress();                       // Retrieve device's short address
  sprintf(shortAddress, "%02X%02X", currentShortAddress[1], currentShortAddress[0]);        // Format the short address
  Serial.print(F("Short Address: "));                         // Display
  Serial.println(shortAddress);                               // Display the short address
  display.printFixed(0, 16, shortAddress, STYLE_NORMAL);      // Display the short address
  Serial.println("Setup complete");                           // Display the end of the setup function
}



void loop(){                                                  // Function to process data
  DW1000Ranging.loop();                                       // Continuously process DW1000 range data

  #ifdef IS_ANCHOR                                            // Check if device is an anchor
  if((millis() - lastUpdateTime) > updateInterval){           // Check that the update interval has passed
    sendToBackend();                                          // Send data to backend
    lastUpdateTime = millis();                                // Update the last update interval
  }
  #endif
}



void newRange(){                                                              // Function when a new distance is reached
  #ifdef IS_ANCHOR                                                            // Check if device is an anchor
    DW1000Device *device = DW1000Ranging.getDistantDevice();                  // Get the tag distance
    uint16_t tag_id = device->getShortAddress();                              // Get the id of the tag
    float distance = device->getRange();                                         // Get the measured distance
    float signal_strength = device->getRxPower();                             // Get the signal srength


    float x_coordinate = calculateX(tag_id, distance);                        // Calculate the x-coordinate
    float y_coordinate = calculateY(tag_id, distance);                        // Calculate the y-coordinate

    saveToFile(tag_id, x_coordinate, y_coordinate, signal_strength);          // Save the data to the text file
  #endif
}



void saveToFile(uint16_t tag_id, float x_coordinate, float y_coordinate, float signal_strength){         // Function to save UWB data to text file
  File file = SPIFFS.open("/uwb_data.txt", FILE_APPEND);                      // Open file to append data
  if(!file){                                                                  // Check if the file opens
    Serial.println("ERROR");                                                  // Display error
    return;                                                                   // Exit
  }

  file.printf("%04X %.2f %.2f %.2f\n", tag_id, x_coordinate, y_coordinate, signal_strength);            // Writing the new entry
  file.close();                                                               // Closing the file
}



void sendToBackend(){                                                       // Function to send text file to the backend

  if(WiFi.status() != WL_CONNECTED){                                        // Check WiFi connection
    Serial.println("ERROR: Backend can't recieve data");                    // Display error
    return;
  }

  File file = SPIFFS.open("/uwb_data.txt", FILE_READ);                      // Open file to read
  if (!file){                                                               // Check if the file opens
    Serial.println("File is not able to be opened.");                       // Display error
    return;
  }

  String jsonData = "[";                                                    // Initiate string array
  bool firstEntry = true;                                                   // Track first entry

  while (file.available()){                                                 // Iterate through each line
    String line = file.readStringUntil('\n');                               // Read each line
    line.trim();                                                            // Remove leading/trailing whitespaces

    if (line.length() > 0){                                                 // Check thtat the line has data
      uint16_t tag_id;                                                      // Variable to store tag ID
      float x_coordinate, y_coordinate;                                     // Variables to store the x and y coordinates

      sscanf(line.c_str(), "%04X %f %f", &tag_id, &x_coordinate, &y_coordinate);            // Parse the line and extract contents


      tag_id = "HIPPO";
      x_coordinate = 700;
      y_coordinate = 700;



      if(!firstEntry){                                                      // Check if it is the first entry
        jsonData += ",";                                                    // Add comma to separate objects
      }

      jsonData += "{";                                                      // Start JSON object
      jsonData += "\"tag_id\":\"" + String(tag_id, 5) + "\",";            // Add tag to object
      jsonData += "\"x_coordinate\":" + String(x_coordinate, 3) + ",";      // Add x-coordinate to object
      jsonData += "\"y_coordinate\":" + String(y_coordinate, 3);            // Add y-coordinate to object
      jsonData += "}";                                                      // Close the JSON object

      firstEntry = false;                                                   // No longer first entry
    }
  }

  file.close();                                                                   // Close the file
  jsonData += "]";                                                                // Close string array

  Serial.println(jsonData);                                                       // Display data being sent

  HTTPClient http;                                                                // Activate http
  http.begin(server);                                                             // Begin HTTP connection to the backend
  http.addHeader("Content-Type", "application/json");                             // Send data as JSON

  int httpResponseCode = http.POST(jsonData);                                     // Send data through HTTP Post

  if(httpResponseCode > 0){                                                       // Check if code was sent
    Serial.println("JSON Data successfully sent");                                // Display that data has sent successfully
  }
  else{                                                                           // Otherwise
    Serial.print("Error in send the data:");                                      // Display error
    Serial.println(httpResponseCode);                                             // Display corresponding http response code
  }

  http.end();                                                                     // End the http connection                                                           
}



float calculateX(uint16_t tag_id, float distance){                          // Function to calculate the x coordinate

}

float calculateY(uint16_t tag_id, float distance){                          // Function to calculate the y coordinate

}



void inactiveDevice(DW1000Device *device){                                  // Function to remove a device
    delete_link(uwb_data, device->getShortAddress());                       // Remove device from the linked list
}



