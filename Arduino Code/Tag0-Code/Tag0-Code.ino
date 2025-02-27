/*
Data transmit Demo
UWB Module Firmware later than 1.1.0 is required

Use 2.0.0   Wire
Use 1.11.7   Adafruit_GFX_Library
Use 1.14.4   Adafruit_BusIO
Use 2.0.0   SPI
Use 2.5.7   Adafruit_SSD1306

*/

// User config          ------------------------------------------

#define UWB_INDEX 0               // Define UWB device index

#define TAG                       // Define Tag
// #define ANCHOR

#define FREQ_850K                 // Define frequency
// #define FREQ_6800K

#define UWB_TAG_COUNT 10          // Define max tag number

// User config end       ------------------------------------------

#include <Wire.h>                 // I2C communications library
#include <Adafruit_GFX.h>         // Graphics library for display
#include <Adafruit_SSD1306.h>     // Library for OLED display
#include <Arduino.h>              // Arduino library

#define SERIAL_LOG Serial         // Define serial output for logging
#define SERIAL_AT mySerial2       // Define serial port for communications

HardwareSerial SERIAL_AT(2);      // Initialize "HardwareSerial" on UART2

#define RESET 16                  // Define reset pin to be pin 16

#define IO_RXD2 18                // Define RX pine for UART2 to be pin 18
#define IO_TXD2 17                // Define TX pin for UART2 to be pin 17

#define I2C_SDA 01                // Define SDA pin for I2C to be pin 01
#define I2C_SCL 02                // Define SCL pin for I2C to be pin 02

Adafruit_SSD1306 display(128, 64, &Wire, -1);       // Initialize OLED display and resolution

void setup(){                                        // Function for setup
    pinMode(RESET, OUTPUT);                         // Reset pin is output
    digitalWrite(RESET, HIGH);                      // Set reset pin to be high

    SERIAL_LOG.begin(115200);                       // Begin serial logging

    SERIAL_LOG.print(F("Hello! ESP32-S3 AT command V1.0 Test"));        // Print message
    SERIAL_AT.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);              // Being serial communications

    SERIAL_AT.println("AT");                                            // Print message???????????
    Wire.begin(I2C_SDA, I2C_SCL);                   // Initialize I2C with SDA and SCL
    delay(1000);                                    // Delay
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){                     // Check that the display initialization was successful
      // Address 0x3C for 128x32
      SERIAL_LOG.println(F("SSD1306 allocation failed"));                // Print error
      for (;;)
        ;                                           // Don't proceed, loop forever
    }
    display.clearDisplay();                         // Clear the display

    logoshow();                                     // Display the log

    // Sending AT commands to configure the device
    sendData("AT?", 2000, 1);
    sendData("AT+RESTORE", 5000, 1);

    sendData(config_cmd(), 2000, 1);
    sendData(cap_cmd(), 2000, 1);

    sendData("AT+SETRPT=0", 2000, 1);
    sendData("AT+SAVE", 2000, 1);
    sendData("AT+RESTART", 2000, 1);
}

long int runtime = 0;                               // Initialize run time to track the last execution
int data_count = 0;                                 // Initialize counter to view that data has transmitted

String response = "";                               // Initialize string to store AT command responses

void loop(){
    if ((millis() - runtime) > 500){                 // Check if 0.5 seconds have passed
        char data_str[80];                           // Character array of size 80
        sprintf(data_str, "AT+DATA=32,UWB_T0_Data:%d", data_count * 2);       // Format the data
        data_count++;                               // Increment the data counter
        sendData(data_str, 2000, 1);                // Send the data
        runtime = millis();                         // Update the last time since last execution
    }

    // put your main code here, to run repeatedly:


    while (SERIAL_LOG.available() > 0){           // Check if there is data available
        SERIAL_AT.write(SERIAL_LOG.read());       // Read from SERIAL_LOG and write to SERIAL_AT
        yield();                                  // Allow for background tasks (aka WiFi handling) to run
    }
    while (SERIAL_AT.available() > 0){            // While data is available
        char c = SERIAL_AT.read();                // Read the incoming character

        if (c == '\r')                            // If character is carriage return
            continue;                             // Continue
        else if (c == '\n' || c == '\r')          // If newline is detected
        {
            SERIAL_LOG.println(response);         // Print the response
            response = "";                        // Clear the response
        }
        else
            response += c;                        // Append character to the response
    }
}

// SSD1306

void logoshow(void){                              // Function to display information on OLED screen
    display.clearDisplay();                       // Clear the display

    display.setTextSize(1);                       // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);          // Draw white text
    display.setCursor(0, 0);                      // Start at top-left corner
    display.println(F("MaUWB DW3000"));           // Print device name

    display.setCursor(0, 20);                     // Start at top-left corner
    // display.println(F("with STM32 AT Command"));

    display.setTextSize(2);                       // Set the text size
    String temp = "";                             // Initialize empty string

#ifdef TAG
    temp = temp + "T" + UWB_INDEX;                // Add device type information
#endif
#ifdef ANCHOR
    temp = temp + "A" + UWB_INDEX;                // Add device type information
#endif
#ifdef FREQ_850K
    temp = temp + "   850k";                      // Add frequency information
#endif
#ifdef FREQ_6800K
    temp = temp + "   6.8M";                      // Add frequency information
#endif
    display.println(temp);                        // Print the devices info

    display.setCursor(0, 40);                     // Move the cursor

    temp = "Total: ";                             // Initialization string
    temp = temp + UWB_TAG_COUNT;                  // Add to the tag count
    display.println(temp);                        // Print the tag count

    display.display();                            // Update the display

    delay(2000);                                  // Delay 2 seconds
}


String sendData(String command, const int timeout, boolean debug){          // Function to output a response from AT
    String response = "";                         // Initialize empty string
    // command = command + "\r\n";

    SERIAL_LOG.println(command);                  // Print the command
    SERIAL_AT.println(command);                   // Send the read character to the SERIAL_LOG

    long int time = millis();                     // Record time

    while ((time + timeout) > millis()){          // Check if enough time has passed
        while (SERIAL_AT.available()){            // If data is available

            // The esp has data so display its output to the serial window
            char c = SERIAL_AT.read();            // Read the next character.
            response += c;                        // Append to response
        }
    }

    if (debug){                                   // Check if debug is enabled
        SERIAL_LOG.println(response);             // Print the response
    }
    return response;                              // Return the response
}


String config_cmd(){                              // Function to configure devices
    String temp = "AT+SETCFG=";                   // AT initialization string

    // Set device id
    temp = temp + UWB_INDEX;                      // Setting the device id

    // Set device role
#ifdef TAG
    temp = temp + ",0";                           // Set as Tag
#endif
#ifdef ANCHOR
    temp = temp + ",1";                           // Set as Anchor
#endif

    // Set frequence 850k or 6.8M
#ifdef FREQ_850K
    temp = temp + ",0";                           // Frequency to 850k
#endif
#ifdef FREQ_6800K
    temp = temp + ",1";                           // Frequency to 6.8M
#endif

    temp = temp + ",1";                           // Set range filter
    return temp;
}


String cap_cmd(){                                 // Function for tag capacity
    String temp = "AT+SETCAP=";                   // AT initialization string

    // Set Tag capacity
    temp = temp + UWB_TAG_COUNT;                  // Increment tag count

    //  Time of a single time slot
#ifdef FREQ_850K
    temp = temp + ",25,1";
#endif
#ifdef FREQ_6800K
    temp = temp + ",10";
#endif

    return temp;
}