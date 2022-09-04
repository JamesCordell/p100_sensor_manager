#include <ArduinoJson.h>
#include <Adafruit_MAX31865.h>
#include <EEPROM.h>
#include "SPI.h"

#include <Arduino_CRC32.h>

Arduino_CRC32 crc32;

// use hardware SPI, just pass in the CS pin
Adafruit_MAX31865 max_1 = Adafruit_MAX31865(10);
Adafruit_MAX31865 max_2 = Adafruit_MAX31865(9);
// The value of the Rref resistor. Use 430.0!
/*
#define RREF1 432.1
#define RREF2 429.2
*/
// Roof array pair
/*
#define RREF1 427.2
#define RREF2 430.9
/*/

unsigned char id1 = 1;
unsigned char id2 = 2;
float rref1 = 430.0f;
float rref2 = 430.0f;
max31865_numwires_t wireNum1 =  MAX31865_3WIRE;
max31865_numwires_t wireNum2 =  MAX31865_3WIRE;

void setup() {

  if (EEPROM.read(0) == 255) { // First boot after flash set defaults.
    EEPROM.put(0, rref1); // 4 bytes for a float
    EEPROM.put(4, rref2);
    EEPROM.put(8, wireNum1);
    EEPROM.put(12, wireNum2);
    EEPROM.put(16, id1);
    EEPROM.put(17, id2);
  } else {  // Else load settings.
    EEPROM.get(0, rref1); // 4 bytes for a float
    EEPROM.get(4, rref2);
    EEPROM.get(8, wireNum1);
    EEPROM.get(12, wireNum2);
    EEPROM.get(16, id1);
    EEPROM.get(17, id2);
  }

  Serial.begin(115200,SERIAL_8N1);
  SPI.begin();
  max_1.begin(wireNum1);  // set to 2WIRE, 3WIRE or 4WIRE as necessary
  max_2.begin(wireNum2);
}

String inData;

void loop() {
  
    while (Serial.available())
    {
        char recieved = Serial.read();
        inData += recieved;

        // Process message when new line character is recieved
        if (recieved == '\n')
        {
            Serial.print(inData);
            process(inData);
            inData = ""; // Clear recieved buffer
        }
    }
}


void process(String inData)
{
    //Serial.println(content);
    switch(inData[0]) {
    case 'h':
       print_help();
    case 'g':
       print_eeprom();
       break;
    case 's': {
      String ref = inData.substring(inData.indexOf(' ')+1 , inData.indexOf('=')) ;

      if (ref == "RREF1") {
        float rref = inData.substring(inData.indexOf('=')+1 , inData.length()).toFloat();
        EEPROM.put(0, rref);
        rref1 = rref;
      }
      else if (ref == "RREF2") {
        float rref = inData.substring(inData.indexOf('=')+1 , inData.length()).toFloat();
        EEPROM.put(4, rref);
        rref2 = rref;
      }
      else if (ref == "WIRE1") {
        int rref = inData.substring(inData.indexOf('=')+1 , inData.length()).toInt();
        wireNum1 = (max31865_numwires_t)rref;
        EEPROM.put(8, wireNum1);
        max_1.begin(wireNum1);  // set to 2WIRE, 3WIRE or 4WIRE as necessary
      }
      else if (ref == "WIRE2") {
        int rref = inData.substring(inData.indexOf('=')+1 , inData.length()).toInt();
        wireNum2 = (max31865_numwires_t)rref;
        EEPROM.put(12, wireNum2);
        max_2.begin(wireNum2);
      }
      else if (ref == "ID1") {
        id1 = inData.substring(inData.indexOf('=')+1 , inData.length()).toInt();
        EEPROM.put(16, id1);
      }
      else if (ref == "ID2") {
        id2 = inData.substring(inData.indexOf('=')+1 , inData.length()).toInt();
        EEPROM.put(17, id2);
      }
      print_eeprom();
      break;
    }
    default:

    String s;
    s.reserve(255);
    String t1= String(max_1.temperature(100, rref1));
    String r1= String(max_1.readRTD());
    String t2= String(max_2.temperature(100, rref2));
    String r2= String(max_2.readRTD());
  
    // Check and print any faults
    uint8_t fault1 = max_1.readFault();
    uint8_t fault2 = max_2.readFault();

    String f1 = "";
    if (fault1) {
      f1 = String(fault1);
      max_1.clearFault();
    }

    String f2 = "";
    if (fault2) {
      f2 = String(fault2);
      max_2.clearFault();
    }

    const int capacity = JSON_ARRAY_SIZE(6) + 2*JSON_OBJECT_SIZE(2);
    StaticJsonDocument<capacity> doc;
    doc["t" + String(id1)] = t1;
    doc["r1"] = r1;
    doc["t" + String(id2)] = t2;
    doc["r2"] = r2;
    doc["f1"] = f1;
    doc["f2"] = f2;
    char output[128];
    serializeJson(doc,output);
    serializeJson(doc, Serial);
    } // switch
}

void print_eeprom() {
       Serial.println("Firmware version 0.1 OpenSolarControl created by James Cordell");
       Serial.print("RREF1:");
       EEPROM.get(0, rref1);
       Serial.println(rref1 , 3);
       Serial.print("RREF2:");
       EEPROM.get(4, rref2);
       Serial.println(rref2 , 3);
       Serial.print("WIRE1:");
       EEPROM.get(8, wireNum1);
       Serial.println(wireNum1);
       Serial.print("WIRE2:");
       EEPROM.get(12, wireNum2);
       Serial.println(wireNum2);
       EEPROM.get(16, id1);
       Serial.print("ID1:");
       Serial.println(id1);
       EEPROM.get(17, id2);
       Serial.print("ID2:");
       Serial.println(id2);
}

void print_help() {

Serial.println("Welcome to OpenSolar sensors, press enter to get sensor readings.");
Serial.println("Type \"g\" to get all settings");
Serial.println("Enter \"s RREF1=430.0\" or RREF2 to update calibration setting");
Serial.println("Enter \"s WIRE1=0\" for 2 and 4 wire or WIRE=1 for three wire probes.");
Serial.println("Enter \"s ID1=3\" or ID2=4 this will set the number of the sensor for example t3 in the JSON output. So sensors can have unique numbers.");

}
