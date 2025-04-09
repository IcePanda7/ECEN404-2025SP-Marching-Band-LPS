
// User config          ------------------------------------------

#define UWB_INDEX 0 //change this for anchor number

#define ANCHOR

#define UWB_TAG_COUNT 10

// User config end       ------------------------------------------

#include <Wire.h>
#include <Arduino.h>

#define SERIAL_LOG Serial
#define SERIAL_AT mySerial2

HardwareSerial SERIAL_AT(2);

// ESP32S3
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

    sendData("AT?", 2000, 1);
    sendData("AT+RESTORE", 5000, 1);
    sendData(config_cmd(), 2000, 1);
    sendData(cap_cmd(), 2000, 1);
    sendData("AT+SETRPT=1", 2000, 1);
    sendData("AT+SAVE", 2000, 1);
    sendData("AT+RESTART", 2000, 1);
}

long int runtime = 0;

String response = "";
//String rec_head = "AT+RANGE";

void loop()
{

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
