// User config          ------------------------------------------
 
#define UWB_INDEX 0 //set tag number
 
#define TAG
// #define ANCHOR
 
//#define FREQ_850K
#define FREQ_6800K
 
#define UWB_TAG_COUNT 10
 
// User config end       ------------------------------------------
 
#include <Wire.h>
#include <Arduino.h>
 
#define SERIAL_LOG Serial
#define SERIAL_AT mySerial2
 
HardwareSerial SERIAL_AT(2);
 
#define RESET 13
#define IO_RXD2 2
#define IO_TXD2 15



void setup()
{
    pinMode(RESET, OUTPUT);
    digitalWrite(RESET, HIGH);
 
    SERIAL_LOG.begin(115200);
 
    SERIAL_LOG.print(F("Hello! ESP32-S3 AT command V1.0 Test"));
    SERIAL_AT.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);
 
    SERIAL_AT.println("AT");
    delay(1000);

    sendData("AT?", 2000, 1);
    sendData("AT+RESTORE", 5000, 1);
 
    sendData(config_cmd(), 2000, 1);
    sendData(cap_cmd(), 2000, 1);
 
    sendData("AT+SETRPT=0", 2000, 1);
    sendData("AT+SAVE", 2000, 1);
    sendData("AT+RESTART", 2000, 1);
}
 
long int runtime = 0;
int data_count = 0;
 
String response = "";
 
void loop()
{
    if ((millis() - runtime) > 500)
    {
        char data_str[80];
        sprintf(data_str, "AT+DATA=32,UWB_T0_Data:%d", data_count * 2);
        data_count++;
        sendData(data_str, 2000, 1);
        runtime = millis();
    }
 
    // put your main code here, to run repeatedly:
    while (SERIAL_LOG.available() > 0)
    {
        SERIAL_AT.write(SERIAL_LOG.read());
        yield();
    }
    while (SERIAL_AT.available() > 0)
    {
        char c = SERIAL_AT.read();
 
        if (c == '\r')
            continue;
        else if (c == '\n' || c == '\r')
        {
            SERIAL_LOG.println(response);
 
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
#ifdef TAG
    temp = temp + ",0";
#endif
#ifdef ANCHOR
    temp = temp + ",1";
#endif
 
    // Set frequence 850k or 6.8M
#ifdef FREQ_850K
    temp = temp + ",0";
#endif
#ifdef FREQ_6800K
    temp = temp + ",1";
#endif
 
    // Set range filter
    temp = temp + ",1";
 
    return temp;
}
 
String cap_cmd()
{
    String temp = "AT+SETCAP=";
 
    // Set Tag capacity
    temp = temp + UWB_TAG_COUNT;
 
    //  Time of a single time slot
#ifdef FREQ_850K
    temp = temp + ",25,1";
#endif
#ifdef FREQ_6800K
    temp = temp + ",10";
#endif
 
    return temp;
}
