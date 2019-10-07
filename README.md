# MH-Z19b
ESP32 with CO2-Sensor (MH-Z19B) and publish data on mydevice Cayenne

![Cayenne](/Cayenne_Reporting.JPG?raw=true "Graph")

## Hardware:
1) Lolin D32 (with ESP32 4MB and WIFI)      https://wiki.wemos.cc/products:d32:d32
2) MH-Z19B - CO2 - Sensor                   https://www.winsen-sensor.com/sensors/co2-sensor/mh-z19b.html

## Wireing:

| MH-Z19b |   |    Lolin D32  |
| --- | -- | -----------|
| GND  | <-> | GND |
| VI V |  <->| BAT |
| RX   | <-> | SDA Pin 21 |
| TX   | <-> | SCL Pin 22 |

## Implementation
- Registration at https://accounts.mydevices.com 
- Add new Device -> select "Bring your own Thing"
- Copy the credentials MQTT - Name / Password and Device-ID from Webpage into the code
- flash and run the code -> Cayonne will create channals automatically
- create a Slider on channel 4 to manipulate the transfer frequency (new Widget - Slider Controller)
  - Data Analog = Acturator / Unit = Anlalog / Channel = 4 / Min = 5 / Max = 60
