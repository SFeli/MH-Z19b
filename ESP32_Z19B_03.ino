/*
 * zweiter Versuch mit Z19B CO2 - Daten auszulesen
 * GND / BAT an einem Stecker
 * 21 und 22 an dem anderen Stecker für RX/TX
 * Vorlage BasicUsage von GitHub: WifWaf/MH-Z19
 * 
 * CAYENNE - senden von Daten Temperatur und CO2
 * Output per WLAN an CAYENNE to MyDevice over WLAN
 * 
 * Version 3 wihtout Password and credentials to publish on guithub
 */

#include <Arduino.h>
#include "MHZ19.h"               // include main library for MH-Z19B
#include "SHTSensor.h"
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP32.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Wire.h>               // This library allows you to communicate with I2C

// WiFi private network credentials.
char ssid[] = "ssid";
char wifiPassword[] = "pw";

// Cayenne authentication info. This can be obtained from the Cayenne Dashboard.
char username[] = "cayenne user";
char password[] = "cayenne pw";
char clientID[] = "cayenne client";

char msg[50];                   // Needed for CAYENNE - Communication
char myVersion[4];              // needed for MH-Z19B

#define RX_PIN 21                                          // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 22                                          // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                      // Native to the sensor (do not change)

MHZ19 myMHZ19;                                             // Constructor for MH-Z19 class
HardwareSerial mySerial(1);                                // ESP32 Example

unsigned long getDataTimer = 0;                            // Variable to store timer interval

void setup()
{
    Serial.begin(115200);                                   // For ESP32 baudarte is 115200 etc.
    mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);   // ESP32 Example
    myMHZ19.begin(mySerial);                      // *Important, Pass your Stream reference

//     MH-Z19 - Communication information       *Shows communication between MHZ19 and Device.
//     myMHZ19.printCommunication(true, true);    // use printCommunication(true, false) to print as HEX
                                                                 
//     MH-Z19 - Autocalibration
//       autoCalibration(false)       - turns auto calibration OFF. (automatically sent before defined period elapses)
//       autoCalibration(true)        - turns auto calibration ON - DEFAULT is true
//       autoCalibration(true, 12)    - turns autocalibration ON and calibration period to 12 hrs (maximum 24hrs).
       myMHZ19.autoCalibration(true);                         
    
//     MH-Z19 - setRange
//       setRange(value) - set range to value (advise 2000 or 5000). - DEFAULT is 2000
       setRange(2000);                           // Set Range 2000 using a function, see below (disabled as part of calibration)

//     MH-Z19 - calibrateZero
//       calibrateZero() - request zero calibration

//     myMHZ19.setSpan(2000);                               // Set Span 2000
//     myMHZ19.recoveryReset();                             // Recovery Reset

//    Primary Information block
      Serial.println("ProgrammID: ESP32_Z19_02 von MiniPC");

//    getVersion(char array[]) returns version number to the argument. The first 2 char are the major 
//    version, and second 2 bytes the minor version. e.g 02.11 
      myMHZ19.getVersion(myVersion);
      Serial.print("\nFirmware Version: ");
      for(byte i = 0; i < 4; i++)
      {
         Serial.print(myVersion[i]);
         if(i == 1)
         Serial.print(".");    
      }
      Serial.println("");

      Serial.print("Range: ");
      Serial.println(myMHZ19.getRange());   
      Serial.print("Background CO2: ");
      Serial.println(myMHZ19.getBackgroundCO2());
      Serial.print("Temperature Cal: ");
      Serial.println(myMHZ19.getTempAdjustment());
      Serial.println("-------------------------------");
      
      Cayenne.begin(username, password, clientID, ssid, wifiPassword);   // WiFi aufsetzen 
}

void loop(){
    StaticJsonDocument<400> doc;
    JsonObject root = doc.to<JsonObject>();
    if (millis() - getDataTimer >= 2000)                    // Check if interval has elapsed (non-blocking delay() equivilant)
    {
        int CO2 = myMHZ19.getCO2(true, true);               // Request CO2 (as ppm) unlimimted value, new request
        Serial.print("CO2 (ppm): ");                      
        Serial.println(CO2);                                
        Cayenne.virtualWrite(0, CO2, "co2", "ppm");         // "co2" and "ppm" required by Cayenne Channel
        root["CO2"] = CO2;                                  // Ref Datatypes on https://community.mydevices.com/t/data-types-for-cayenne-mqtt-api/3714
        
        float Temp = myMHZ19.getTemperature(true, false);  // decimal value, not new request                 
        Serial.print("Temperature (C): ");                  
        Serial.println(Temp);
        Cayenne.virtualWrite(1, Temp, "temp", "c");
        root["Temperature"] = Temp;

        serializeJson(root, msg);
        
/*      Serial.println("------------------------------");
        Serial.print("Transmittance: ");
        Serial.print(myMHZ19.getTransmittance(), 7);         // 7 decimals for float/double maximum Arduino accuracy
        Serial.println(" %");
        
        Serial.print("Temp Offset: ");
        Serial.print(myMHZ19.getTemperatureOffset(), 2);
        Serial.println(" C");        */

/*      double adjustedCO2 = myMHZ19.getCO2Raw();
        Serial.print("Raw CO2: ");
        Serial.println(adjustedCO2, 0);
    
        adjustedCO2 = 6.60435861e+15 * exp(-8.78661228e-04 * adjustedCO2);      // Exponential equation for Raw & CO2 relationship
        Serial.print("Adjusted CO2: ");
        Serial.print(adjustedCO2, 2);
        Serial.println(" ppm");   */
        
        getDataTimer = millis();                            // Update interval
    }
}

void setRange(int range)
{
    Serial.println("Setting range..");
    myMHZ19.setRange(range);                                               // request new range write

    if ((myMHZ19.errorCode == RESULT_OK) && (myMHZ19.getRange() == range)) //RESULT_OK is an alias from the library,
        Serial.println("Range successfully applied");
    else
    {
        Serial.print("Failed to set Range! Error Code: ");
        Serial.println(myMHZ19.errorCode);          // Get the Error Code value
    }
}

void verifyRange(int range)
{
    Serial.println("Requesting new range.");

    myMHZ19.setRange(range);                             // request new range write

    if (myMHZ19.getRange() == range)                     // Send command to device to return it's range value.
        Serial.println("Range successfully applied.");   // Success
    else
        Serial.println("Failed to apply range.");        // Failed
}
