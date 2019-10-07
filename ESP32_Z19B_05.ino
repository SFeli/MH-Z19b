/*  für GuiHub ohne PW und Schlüssel 
 * fünfter Versuch mit Z19B CO2 - Daten auszulesen
 *     mit Sendefrequenintervall über Slider in myDevice
 *     MH-Z19B mißt alle 5 Sekunden
 * Vorlage BasicUsage von GitHub: WifWaf/MH-Z19
 * Funktion:
 *     Alle 2 Sekunden - Daten (CO2 und Temperatur) messen und
 *     über WLAN nach myDevice senden. 
 * Hardware:
 *     ESP32 Lolin D32 
 *     Sensor MH-Z19B 0--5000 ppm
 * Verkabelung:
 *     GND an GND / USB an Vin einem Stecker
 *     Pin 21 (SDA) und Pin 22 (SCL) an TX bzw. RX 
 * MH-Z19B - Funktionen:    
 *     myMHZ19.printCommunication(boolean, boolean) .. (true/false) zur Fehlersuche (true/false) als HEX-Code
 *     myMHZ19.autoCalibration(bollean)             .. (true/false) auf 400 ppm calibrieren
 *     myHZZ19.setRange(value)                      .. (2000/5000) Empfindlichkeit
 *     myMHZ19.setSpan(num)                         ..
 *     myMHZ19.recoveryReset()                      .. 
 *     myMHZ19.getVersion(char array[])             .. returns version number e.g 02.11 
 *     myMHZ19.getRange()
 *     myMHZ19.getBackgroundCO2()
 *     myMHZ19.getTempAdjustment()
 * MyDevice - Einstellungen:
 *     Neues Device (entweder Generic ESP8266 oder Bring Your Own Device)
 *     MQTT Username / MQTT Passwort und Client ID in das Strech kopieren
 *     Neues Custom Widget anlegen - Slider Controller Widget
 *     Data Analog = Actuator / Unit = Analog / Channel = 4 / Min = 5 / Max = 60
 */

#include <Arduino.h>
#include "MHZ19.h"              // include main library for MH-Z19B
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP32.h>
#include <Wire.h>               // This library allows you to communicate with I2C
#define RX_PIN 21               // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 22               // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600           // Native to the sensor (do not change)

// WiFi private network credentials.
char ssid[] = "WLAN";
char wifiPassword[] = "WLAN-PW";

// Cayenne authentication info. This can be obtained from the Cayenne Dashboard.
char username[] = "Cayenne-MQTT-User";
char password[] = "Cayenne-MQTT-Passwort";
char clientID[] = "ClientID";

char msg[50];                   // needed for CAYENNE - Communication
char myVersion[4];              // needed for MH-Z19B
int frequenz;                   // Frequenz der Übertragung (min 5 Sekunden, da nur alle 5 Sekunden gemessen wird.)
unsigned long getDataTimer = 0; // Variable to store timer interval

MHZ19 myMHZ19;                  // Constructor for MH-Z19 class
HardwareSerial mySerial(1);     // ESP32 Example

void setup()
{
    Serial.begin(115200);          // For ESP32 baudarte is 115200 etc.
    mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);   // ESP32 Example
    delay(500);
    myMHZ19.begin(mySerial);       // *Important, Pass your Stream reference
    delay(500);
    myMHZ19.autoCalibration(true);                         
    setRange(2000);                // set Range 2000 using a function, see below (disabled as part of calibration)
    verifyRange(2000);             // Check if 2000 is set as range
//  calibrateZero()                // request zero calibration
//  myMHZ19.setSpan(2000);         // set Span 2000
//  myMHZ19.recoveryReset();       // Recovery Reset

//  Primary Information block
    Serial.println("ProgrammID: ESP32_Z19_04 von MiniPC");
    myMHZ19.getVersion(myVersion);
    Serial.print("Firmware Version: ");
    for(byte i = 0; i < 4; i++)
    {
       Serial.print(myVersion[i]);
       if(i == 1)
       Serial.print(".");    
    }
    Serial.println("");
    Serial.print("Background CO2: ");
    Serial.println(myMHZ19.getBackgroundCO2());
    Serial.print("Temperature Cal: ");
    Serial.println(myMHZ19.getTempAdjustment());
    Serial.println("-------------------------------");
    Cayenne.begin(username, password, clientID, ssid, wifiPassword);   // WiFi aufsetzen
    WiFi.setHostname("MH-Z19B_04");
    Serial.println(WiFi.getHostname());
}

void loop(){
    Cayenne.loop();
    if (millis() - getDataTimer >= (frequenz * 1000))       // Check if interval has elapsed (non-blocking delay() equivilant)
    {
        int CO2 = myMHZ19.getCO2(true, true);               // Request CO2 (as ppm) unlimimted value, new request
        Serial.print("CO2 (ppm): ");                      
        Serial.println(CO2);                                
        Cayenne.virtualWrite(0, CO2, "co2", "ppm");         // "co2" and "ppm" required by Cayenne Channel
       // Ref Datatypes on https://community.mydevices.com/t/data-types-for-cayenne-mqtt-api/3714
        
        float Temp = myMHZ19.getTemperature(true, false);   // decimal value, not new request                 
        Serial.print("Temperature (C): ");                  
        Serial.println(Temp);
        Cayenne.virtualWrite(1, Temp, "temp", "c");
       
/*      Serial.println("------------------------------");
        Serial.print("Transmittance: ");
        Serial.print(myMHZ19.getTransmittance(), 7);     // 7 decimals for float/double maximum Arduino accuracy
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

        getDataTimer = millis();                   // Update interval
    }
}

void setRange(int range)
{
    Serial.println("Setting range..");
    myMHZ19.setRange(range);                        // request new range write
    if ((myMHZ19.errorCode == RESULT_OK) && (myMHZ19.getRange() == range))     //RESULT_OK is an alias from the library,
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
    myMHZ19.setRange(range);                       // request new range write
    if (myMHZ19.getRange() == range)               // Send command to device to return it's range value.
    {
        Serial.print("Range :");
        Serial.print(range); 
        Serial.println(" successfully applied.");  // Success
    }   else
        Serial.println("Failed to apply range.");  // Failed
}

// This function will be called every time a Dashboard widget writes a value to Virtual Channel 4.
CAYENNE_IN(4)
{
     frequenz = getValue.asInt();
     Serial.print("Sendefrequenz :");
     Serial.print(frequenz);
     Serial.println(" Sekunden");
}
