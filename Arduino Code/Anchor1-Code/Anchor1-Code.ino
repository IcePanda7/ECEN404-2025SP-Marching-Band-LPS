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

#define UWB_INDEX 1           // Defining the UWB index for device

// #define TAG
#define ANCHOR                // Defining the device is an Anchor

#define FREQ_850K             // Set the frequency for communications to 850kHz
// #define FREQ_6800K

#define UWB_TAG_COUNT 10      // Define the number of tags connect to the anchor          XXXXX

// User config end       ------------------------------------------

#include <Wire.h>               // Library for I2C communications
#include <Adafruit_GFX.h>       // Library for display
#include <Adafruit_SSD1306.h>   // Library for OLED display
#include <Arduino.h>            // Library for Arduino functions

#define SERIAL_LOG Serial       // Define Serial Log
#define SERIAL_AT mySerial2     // Define Serial for AT commands

HardwareSerial SERIAL_AT(2);    // Hardware serial 2 for AT commands

#define RESET 16                // Set the RESET pin to pin 16

#define IO_RXD2 18              // Set the RXD2 pin to pin 18
#define IO_TXD2 17              // Set the TXD2 pin to pin 17

#define I2C_SDA 39              // Set the SDA pin to pin 39
#define I2C_SCL 38              // Set the SCL pin to pin 38

Adafruit_SSD1306 display(128, 64, &Wire, -1);     // Initialize OLED display with resoultion

void setup(){                   // Function for setup
    pinMode(RESET, OUTPUT);     // Set the RESET pin as output
    digitalWrite(RESET, HIGH);  // Set the RESET pin to HIGH to initialize UWB

    SERIAL_LOG.begin(115200);   // Start serial communications

    SERIAL_LOG.print(F("Hello! ESP32-S3 AT command V1.0 Test"));    // Print message
    SERIAL_AT.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);          // Initialize AT commands serial communications

    SERIAL_AT.println("AT");            // Check connection with UWB by sending "AT" command
    Wire.begin(I2C_SDA, I2C_SCL);       // Initialize I2C for OLED display
    delay(1000);                        // Delay 1 second
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))     // Check if OLED is connected
    { // Address 0x3C for 128x32
        SERIAL_LOG.println(F("SSD1306 allocation failed"));       // Print error message
        for (;;)
            ; // Don't proceed, loop forever
    }
    display.clearDisplay();             // Clear the display

    logoshow();                         // Show logs on OLED

    sendData("AT?", 2000, 1);             // Query module status
    sendData("AT+RESTORE", 5000, 1);      // Restore settings

    sendData(config_cmd(), 2000, 1);      // Send configuration command for device setup
    sendData(cap_cmd(), 2000, 1);         // Send capacity command to set tag capacity

    sendData("AT+SETRPT=0", 2000, 1);     // Set reporting mode
    sendData("AT+SAVE", 2000, 1);         // To save configuration
    sendData("AT+RESTART", 2000, 1);      // To restart the UWB
}

long int runtime = 0;                     // Initialize variable to track runtime
int data_count = 0;                       // Initialize variable to track data # sent

String response = "";                     // Initialize empty string to store response

void loop(){
    if ((millis() - runtime) > 1000)            // Every second
    {
        char data_str[80];                      // Initialize character array of size 80
        sprintf(data_str, "AT+DATA=32,UWB_A0_Data:%d", data_count * 2 + 1);     // Formatting the data string
        data_count++;                           // Increment data count
        sendData(data_str, 2000, 1);            // Send data to UWB
        runtime = millis();                     // Update runtime
    }

    // put your main code here, to run repeatedly:
    while (SERIAL_LOG.available() > 0){          // Check if data is available on log
        SERIAL_AT.write(SERIAL_LOG.read());      // Forward the data to AT serial port
        yield();                                 // Yield
    }
    while (SERIAL_AT.available() > 0){           // Check if data is availale on UWB
        char c = SERIAL_AT.read();               // Read the character

        if (c == '\r')                           // Check if the character is a carriage return
            continue;
        else if (c == '\n' || c == '\r'){        // Check if the character is new line or carriage return
            SERIAL_LOG.println(response);        // Print the response
            response = "";                       // Reset the repsonse string
        }
        else
            response += c;                       // Append character to response string
    }
}

// SSD1306

void logoshow(void){
    display.clearDisplay();              // Clear the display

    display.setTextSize(1);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.println(F("MaUWB DW3000"));  // Print message

    display.setCursor(0, 20); // Start at top-left corner
    // display.println(F("with STM32 AT Command"));

    display.setTextSize(2);              // Set text size

    String temp = "";                    // Initialize a temp string

#ifdef TAG                               // Check if define is a Tag
    temp = temp + "T" + UWB_INDEX;       // Add T before UWB index
#endif
#ifdef ANCHOR                           // Check if define is a anchor
    temp = temp + "A" + UWB_INDEX;      // Add A before UWB index
#endif
#ifdef FREQ_850K                        // Check if device communicating at 850kHz
    temp = temp + "   850k";            // Add 850k to temp string
#endif
#ifdef FREQ_6800K                       // Check if device communicating at 6.8MHz
    temp = temp + "   6.8M";            // Add 6.8 MHz to temp string
#endif
    display.println(temp);              // Display temp string

    display.setCursor(0, 40);           // Move cursor down

    temp = "Total: ";                   // Initialize new temp
    temp = temp + UWB_TAG_COUNT;        // Add the tag count to the temp string
    display.println(temp);              // Dispaly temp message

    display.display();                  // Update OLED for message

    delay(2000);                        // Delay 2 seconds
}

String sendData(String command, const int timeout, boolean debug){      // Function to send data to UWB
    String response = "";         // Initialize response string
    // command = command + "\r\n";

    SERIAL_LOG.println(command);  // Print command to log
    SERIAL_AT.println(command); // send the read character to the SERIAL_LOG

    long int time = millis();     // Retrieve time

    while ((time + timeout) > millis()){        // Wait for timeout+time period
        while (SERIAL_AT.available()){          // If data available in UWB

            // The esp has data so display its output to the serial window
            char c = SERIAL_AT.read(); // read the next character.
            response += c;             // Append character to repsonse string
        }
    }

    if (debug){                       // Check for debug mode
        SERIAL_LOG.println(response); // Print response to log
    }
    return response;                  // Return the response string
}

String config_cmd(){                // Function to generate the configuration
    String temp = "AT+SETCFG=";     // Initialize configuration command

    // Set device id
    temp = temp + UWB_INDEX;        // Add UWB index to the command

    // Set device role
#ifdef TAG                          // If device defined as tag
    temp = temp + ",0";             // Set device as Tag
#endif
#ifdef ANCHOR                       // If device defined as anchor
    temp = temp + ",1";             // Set device as Anchor
#endif

    // Set frequence 850k or 6.8M
#ifdef FREQ_850K                    // If frequency defined at 850kHz
    temp = temp + ",0";             // Set frequency as 850kHz
#endif
#ifdef FREQ_6800K                   // If frequency defined at 6.8MHz
    temp = temp + ",1";             // Set frequency as 6.8MHz
#endif

    // Set range filter
    temp = temp + ",1";

    return temp;                    // Return the configuration
}

String cap_cmd(){                   // Function to generate capability
    String temp = "AT+SETCAP=";     // Initialize capability command

    // Set Tag capacity
    temp = temp + UWB_TAG_COUNT;

    //  Time of a single time slot
#ifdef FREQ_850K                // If frequency is defined at 850kHz
    temp = temp + ",25,1";      // Set time slot for 850kHz
#endif
#ifdef FREQ_6800K               // If frequency is defined at 6.8MHz
    temp = temp + ",15,1";      // Set time slot for 6.8MHz
#endif

    return temp;                // Return the temp
}