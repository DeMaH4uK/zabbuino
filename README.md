# Zabbuino
An Zabbix agent firmware for Arduino (**AVR only**)

Actual release is v1.0.0. 

**You can help to the project** by providing hardware to testing and integrating. Or yo can just donate for it. Contact to me via email: [zbx.sadman@gmail.com](mailto://zbx.sadman@gmail.com)

Implemented:
- A few Zabbix agent commands;
- Wraps a lot of Arduino Language functions;
- OneWire and I2C bus scaning to detect sensors ID or adresses;
- Network DHCP and static IP support;
- Remote configuring & rebooting, system protection;
- Storing system setting in EEPROM;
- AVR WatchDog feature support;
- MCU and runtime metrics (current/min/max VCC, current/min RAM, uptime, MCU name) obtaining;
- Support WS5100 and ENC25J60 network modules;
- Support one or more DS18X20 thermometer;
- Support DHT11/21/22/33/44 or AM2301/2302 humidity and temperature sensors;
- Support SHT2X humidity and temperature sensors serie;
- Support BMP085/180, BMP280/BME280 pressure and temperature sensors;
- Support BH1750 light sensor;
- Support incremental Encoder (on interrupt's pin);
- Support any devices that can be used with hardware interrupt - tilt switches, dry contacts, water flow sensor, and so;
- Support ACS712-XX sensor;
- Support HC-SR04 ultrasonic ranging module;
- Support any other analog or digital sensor via `analogread` /`digitalread` commands;
- Support indicators, that connected to MAX7219, 8x8 Led matrix for example;
- Support simple I2C devices (expanders, digital linear potentiometers, etc.);
- Support One- or Two- (and maybe Four-) lines LCD Character displays with PC8574 I2C expander;
- Support any actuators or indicators via `digitalwrite` command;
- Support WS2801 Led stripe and any indicators on shift registers via extended `shiftout` command;
- Simulate varuious vendor's IR transmitters.


Minimum requirements: 
- Arduino board with ATMega328 & ENC28J60 / W5100 Ethernet Module.

Manuals:
- In [Russian](https://github.com/zbx-sadman/zabbuino/wiki/Zabbuino-in-Russian);
- Partially translated to [English](https://github.com/zbx-sadman/Zabbuino/wiki/Zabbuino-in-English).

User cases:
- In [Russian](https://github.com/zbx-sadman/Zabbuino/wiki/Zabbuino-User-Cases-in-Russian)
