// http://www.kandrsmith.org/RJS/Misc/Hygrometers/calib_many.html
#ifndef Zabbuino_h
#define Zabbuino_h

#include <Arduino.h>
#include <IPAddress.h>
#include "platforms.h"


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                                 PROGRAMM FEATURES SECTION

                                  Comment #define's below to save RAM and Flash and uncomment to enable some feature 

*/

/****       Network      ****/

// Uncomment to use DHCP address obtaining
//#define FEATURE_NET_DHCP_ENABLE

// Uncomment to force using DHCP even netConfig.useDHCP = false
//#define FEATURE_NET_DHCP_FORCE

/****       Arduino      ****/

// Uncomment to enable Arduino's tone[], noTone[] commands
//#define FEATURE_TONE_ENABLE

// Uncomment to enable Arduino's randomSeed, random[] commands
//#define FEATURE_RANDOM_ENABLE

// Uncomment to enable shiftOut[] command
//#define FEATURE_SHIFTOUT_ENABLE

/****    Other   ****/

// Uncomment to enable external interrupts handling: interrupt.*[] commands
//#define FEATURE_EXTERNAL_INTERRUPT_ENABLE

// Uncomment to enable encoder handling with external interrupts: encoder.*[] commands
//#define FEATURE_ENCODER_ENABLE

/****      1-Wire bus      ****/

// Uncomment to enable 1-Wire common functions: OW.Scan[]
//#define FEATURE_OW_ENABLE

// Uncomment to enable Dallas DS18x20 family functions: DS18x20.*[] commands
//#define FEATURE_DS18X20_ENABLE

/****        I2C bus       ****/

// Note #1: I2C library (Wire.h) takes at least 32bytes of memory for internal buffers
// Note #2: I2C library (Wire.h) activate internal pullups for SDA & SCL pins when Wire.begin() called

// Uncomment to enable I2C commands: I2C.Scan[], I2C.Read[], I2C.Write[], I2C.BitRead[], I2C.BitWrite[]
//#define FEATURE_I2C_ENABLE

// Uncomment to enable BMP pressure sensors functions: BMP.*[] commands
#define FEATURE_BMP_ENABLE
#define SUPPORT_BMP180_INCLUDE
#define SUPPORT_BMP280_INCLUDE
// BME280 is BMP280+Humidity sensor. If you want to get all, uncomment SUPPORT_BMP280_INCLUDE too
#define SUPPORT_BME280_INCLUDE

// Uncomment to enable BH1750 light sensors functions: BH1750.*[] commands
#define FEATURE_BH1750_ENABLE

// LCD connected via PCF8574 I2C expander enable
//#define FEATURE_PCF8574_LCD_ENABLE

//
//#define FEATURE_SHT2X_ENABLE

/****        MicroWire bus       ****/

// Uncomment to enable MAX7219 8x8 led matrix functions: MAX7219.*[] commands
//#define FEATURE_MAX7219_ENABLE

/****    DHT/AM family    ****/

// Uncomment to enable DHT/AM humidity sensors functions: DHT.*[] commands
#define FEATURE_DHT_ENABLE

/****    Ultrasonic    ****/

// HC-SR04 sensor
//#define FEATURE_ULTRASONIC_ENABLE

/****    ACS7XX family    ****/

//#define FEATURE_ACS7XX_ENABLE



/****      System        ****/

// Uncomment to enable AVR watchdog
//                                                                     !!! BEWARE !!!
//                                                     NOT ALL BOOTLOADERS HANDLE WATCHDOG PROPERLY 
//                                                    http://forum.arduino.cc/index.php?topic=157406.0 
// 
// Note: OptiBoot is watchdog compatible and use less flash space that stock bootloader.
// Note: watchdog timeout may be vary for many controllers, see comments to macro WTD_TIMEOUT in zabbuino.h
//#define FEATURE_WATCHDOG_ENABLE

// Uncomment ***only*** if you know all about AREF pin using ***risks***
// // // // #define FEATURE_AREF_ENABLE

// Uncomment to be able to store runtime settings in EEPROM and use its on start
#define FEATURE_EEPROM_ENABLE

// debug only option, must be removed on releasing 
//#define FEATURE_EEPROM_SET_COMMANDS_ENABLE

// Uncomment to force protect (enable even netConfig.useProtection is true) your system from illegal access for change runtime settings and reboots 
//#define FEATURE_PASSWORD_PROTECTION_FORCE

// Uncomment to enable system's command which can be used in system debug process: sys.cmd.count, sys.ram.free and so
#define FEATURE_DEBUG_COMMANDS_ENABLE

// Uncomment to view the debug messages on the Serial Monitor
//#define FEATURE_DEBUG_TO_SERIAL

// Uncomment to enable using time+interrupt for internal metric gathering
#define GATHER_METRIC_USING_TIMER_INTERRUPT



/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                               ALARM SECTION 
*/

// Where is state LED connected
#define PIN_STATE_LED              	                        0x09
// State LED must blink or just be turned on?
#define ON_ALARM_STATE_BLINK            
// Use more blinks to runtime stage indication
//#define ADVANCED_BLINKING

// Turn off state LED blink (no errors found)
// State LED no blink type
#define BLINK_NOPE                 	                        0x00
// State LED blink type with DHCP problem reached (no renew lease or more)
#define BLINK_DHCP_PROBLEM         	                        150 // ~150ms on, ~850ms off
// State LED blink type with Network activity problem (no packets processed for NET_IDLE_TIMEOUT)
#define BLINK_NET_PROBLEM          	                        500 // ~500ms on, ~500ms off


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                            NETWORK MODULE SECTION 
*/

#define NET_DEFAULT_USE_DHCP        	                        false
#define NET_DEFAULT_MAC_ADDRESS     	                        {0xDE,0xAD,0xBE,0xEF,0xFE,0xF4}

#ifdef USE_NETWORK_192_168_0_1
  #define NET_DEFAULT_IP_ADDRESS      	                        {192,168,0,228}
  #define NET_DEFAULT_GATEWAY         	                        {192,168,0,1}
#else
  #define NET_DEFAULT_IP_ADDRESS      	                        {172,16,100,228}
  #define NET_DEFAULT_GATEWAY         	                        {172,16,100,254}
#endif
 
#define NET_DEFAULT_NETMASK           	                        {255,255,255,0}

// How often do ENC28J60 module reinit for more stable network
#define NET_ENC28J60_REINIT_PERIOD  	                        10000UL  // 10 sec
// Network activity timeout (for which no packets processed or no DHCP lease renews finished with success)
#define NET_IDLE_TIMEOUT            	                        60000UL  // 60 sec
// How often do renew DHCP lease
#define NET_DHCP_RENEW_PERIOD       	                        30000UL  // 30 sec
// How often do renew DHCP lease
#define NET_STABILIZATION_DELAY     	                        100L     // 0.1 sec

#ifdef ethernet_h
  #define NET_MODULE_NAME         	                        "W5xxx"
#elif defined UIPETHERNET_H
  #define NET_MODULE_NAME         	                        "ENC28J60"
#endif

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                         SYSTEM CONFIGURATION SECTION 
*/

#define SYS_DEFAULT_PROTECTION      	                        true
// It's just number of "long int" type. Surprise!
#define SYS_DEFAULT_PASSWORD        	                        0x000

// Digital pin which must shorted on the GND for HOLD_TIME_TO_FACTORY_RESET time to save default system setting into EEPROM
#define PIN_FACTORY_RESET           	                        8 
#define HOLD_TIME_TO_FACTORY_RESET  	                        5000L // 5 seconds

// How many secs device may be stay in infinitibe loop before reboot
// Also you can use:
// WDTO_1S
// WDTO_2S
// WDTO_4S - not for all controllers (please reference to avr/wdt.h)
// WDTO_8S - not for all controllers (please reference to avr/wdt.h)
#define WTD_TIMEOUT                   	                        WDTO_8S 

#define SYS_METRIC_RENEW_PERIOD        	                        1000L // 1 sec

// Number of expected arguments of the command
#define ARGS_MAX                    	                        6
// Size of buffer's argument part. All separators and delimiters must be taken into account
#define ARGS_PART_SIZE         	                                50
// Size of buffer's command part
#define CMD_PART_SIZE          	                                25
// The total size of the buffer
#define BUFFER_SIZE                   	                        CMD_PART_SIZE + ARGS_PART_SIZE

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                          AGENT CONFIGURATION SECTION 
*/

#define ZBX_AGENT_DEFAULT_HOSTNAME  	                        "zabbuino.local.net"

// How much bytes will be allocated to hostname store
#define ZBX_AGENT_HOSTNAME_MAXLEN   	                        25

#define ZBX_NOTSUPPORTED_MSG          	                        "ZBX_NOTSUPPORTED"

#define ZBX_AGENT_VERISON             	                        "Zabbuino 1.0.0"


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                          I/O PORTS/PINS PRE-CONFIGURATION SECTION 
*/

// see below: const uint8_t port_protect[PORTS_NUM] = {...}
#if defined (ARDUINO_AVR_DUEMILANOVE) || defined (ARDUINO_AVR_MINI) || defined (ARDUINO_AVR_NANO) || defined (ARDUINO_AVR_NG) || defined (ARDUINO_AVR_PRO) || defined (ARDUINO_AVR_ETHERNET)
#define PORTS_NUM                                               0x05
#elif defined(ARDUINO_AVR_LEONARDO) || defined (ARDUINO_AVR_MICRO) || defined(ARDUINO_AVR_ROBOT_CONTROL) || defined(ARDUINO_AVR_ROBOT_MOTOR) || defined (ARDUINO_AVR_YUN)
#define PORTS_NUM                                               0x07
#elif defined (ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560)
#define PORTS_NUM                                               0x0D
#else // Unknow boards equal to "Arduino Duemilanove"
#define PORTS_NUM 						0x05
#define ARDUINO_AVR_DUEMILANOVE
#endif

/*
  Защитные маски портов ввода/вывода.
  Значение '1' в определенной битовой позиции означает то, что при операциях записи в порт данный бит не будет изменен (находится под защитой). Эта маски
  также применяются для проверки безопасности при операциях изменения состояния определенного пина. Функция isSafePin() проверяет не установлена ли защита
  для конкретного пина.

  Вы можете изменять как сами маски, так и их количество. Однако, количество элементов данного массива должно соответствовать параметру PORTS_NUM.

  Например: Необходимо защитить от изменения pin D13, так как он используется библиотекой Ethernet и его изменение извне приведет к некорректной работе
  сетевого адаптера. В заголовочном файле pins_arduino.h определенном для необходимой платформы находим массив digital_pin_to_bit_mask_PGM. Элемент #13 указывает на
  связь данного пина с портом B (PORTB) и битом 5. Значит, для защиты данного пина необходимо установить в нижележащем массиве port_protect 5-й символ справа
  в строке "B..... // PORTB" в значение '1'.
*/

const uint8_t port_protect[PORTS_NUM] = {
#if defined (ARDUINO_AVR_DUEMILANOVE) || defined (ARDUINO_AVR_MINI) || defined (ARDUINO_AVR_NANO) || defined (ARDUINO_AVR_NG) || defined (ARDUINO_AVR_PRO) || defined (ARDUINO_AVR_ETHERNET)
  B00000000, // not a port
  B00000000, // not a port
  // Pins D10, D11, D12, D13 is protected by setting 2, 3, 4, 5 bits, due its used to SPI (ethernet shield)
  // Pin D9 is used by Timer1 and can't be used for PWM (analogWrite) when system metric was gathered on Timer1 interrupt
  // Bits 6, 7 have not correspondented pins in Arduino Mini Pro / Freeduino 2009
  B11111110, /*     PORTB        
D13 -^    ^- D8    <- pins   */
  B00000000, /*     PORTC 
   ^-A7   ^-A0   <- pins    */
  // Pins D0, D1 is protected by settings 0, 1 bits, due its used to RX/TX lines of UART and make it possible to transmit data to Serial Monitor  
  B00000011  /*     PORTD 
   ^-D7   ^-D0   <- pins    */
#elif defined(ARDUINO_AVR_LEONARDO) || defined (ARDUINO_AVR_MICRO) || defined(ARDUINO_AVR_ROBOT_CONTROL) || defined(ARDUINO_AVR_ROBOT_MOTOR) || defined (ARDUINO_AVR_YUN)
  // check ports settings for your platform
  B00000000, // not a port
  B00000000, // not a port
  B00000000, // PORTB
  B00000000, // PORTC
  B00000000, // PORTD
  B00000000, // PORTE
  B00000000  // PORTF
#elif defined (ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560)
  // check ports settings for your platform
  B00000000, // not a port
  B00000000, // PORTA
  B00000000, // PORTB
  B00000000, // PORTC
  B00000000, // PORTD
  B00000000, // PORTE
  B00000000, // PORTF
  B00000000, // PORTG
  B00000000, // PORTH
  B00000000, // not a port
  B00000000, // PORTJ
  B00000000, // PORTK
  B00000000  // PORTL
#endif
};

/*
   Маски для задания направления ввода/вывода выходов микроконтроллера.
   Значение '1' в определенной битовой позиции означает то, что при инициализации соответствующий биту пин будет установлен в состояние OUTPUT.
   В противном случае он будет оставлен с состоянии по умолчанию для платформы, на которой выполняется программный код.

   Вы можете изменять как сами маски, так и их количество. Однако, количество элементов данного массива должно соответствовать параметру PORTS_NUM.

   Например: Необходимо при инициализации установить пин D8 в режим OUTPUT, а пин D9 оставить в состоянии по умолчанию - INPUT.
   В заголовочном файле pins_arduino.h определенном для необходимой платформы находим массив digital_pin_to_bit_mask_PGM. Элемент #8 связан с битом 0 порта B,
   а элемент #9 с битом 1 того же порта. Значит, для правильной инициализации следует установить в нижележащем массиве port_mode 0-й символ справа
   в строке "B..... // PORTB" в значение '1', а 1-й символ справа в значение '0'

   Будьте внимательны и осторожны. Установка пина в состояние INPUT увеличивает при неаккуратном обращении с устройством вероятность вывода из строя
   соответствующего вывода микроконтроллера.
*/


const uint8_t port_mode[PORTS_NUM] = {
 
  // All bits equal '0' cause setting corresponding pin to INPUT mode
  // All bits equal '1' cause setting corresponding pin to OUTPUT mode
 
#if defined (ARDUINO_AVR_DUEMILANOVE) || defined (ARDUINO_AVR_MINI) || defined (ARDUINO_AVR_NANO) || defined (ARDUINO_AVR_NG) || defined (ARDUINO_AVR_PRO) || defined (ARDUINO_AVR_ETHERNET)
  B00000000, // not a port
  B00000000, // not a port
  // Bits 6, 7 have not correspondented pins in Arduino Mini Pro / Freeduino 2009
  B00111110, /*     PORTB        
D13 -^    ^- D8    <- pins   */
  B11111110, /*     PORTC 
   ^-A7   ^-A0   <- pins    */
  B11111111  /*     PORTD 
   ^-D7   ^-D0   <- pins    */
#elif defined(ARDUINO_AVR_LEONARDO) || defined (ARDUINO_AVR_MICRO) || defined(ARDUINO_AVR_ROBOT_CONTROL) || defined(ARDUINO_AVR_ROBOT_MOTOR) || defined (ARDUINO_AVR_YUN)
  // check ports settings for your platform
  B00000000, // not a port
  B00000000, // not a port
  B11111111, // PORTB
  B11111111, // PORTC
  B11111111, // PORTD
  B11111111, // PORTE
  B11111111  // PORTF
#elif defined (ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560)
  // check ports settings for your platform
  B00000000, // not a port
  B11111111, // PORTA
  B11111111, // PORTB
  B11111111, // PORTC
  B11111111, // PORTD
  B11111111, // PORTE
  B11111111, // PORTF
  B11111111, // PORTG
  B11111111, // PORTH
  B00000000, // not a port
  B11111111, // PORTJ
  B11111111, // PORTK
  B11111111  // PORTL
#endif
};

/*
  Маски для установки состояния выходов микроконтроллера.
  Значение '1' в определенной битовой позиции задает высокое состояние пина при инициализации (см. описание функции Arduino pinMode()). Если маской port_mode
  соответствующий пин определен, как OUTPUT, то его итоговое состояние станет OUTPUT+HIGH. В случае с определением пина, как работающего в режиме INPUT, итоговым
  состоянием будет INPUT_PULLUP.

  Вы можете изменять как сами маски, так и их количество. Однако, количество элементов данного массива должно соответствовать параметру PORTS_NUM.
  Вычисление битов, соотвующих пинам аналогично способам, применяемым в port_protect и port_mode.
*/
const uint8_t port_pullup[PORTS_NUM] = {

  // All bits equal '0' cause do not pull-up corresponding pin
  // All bits equal '1' cause pull-up corresponding pin

#if defined (ARDUINO_AVR_DUEMILANOVE) || defined (ARDUINO_AVR_MINI) || defined (ARDUINO_AVR_NANO) || defined (ARDUINO_AVR_NG) || defined (ARDUINO_AVR_PRO) || defined (ARDUINO_AVR_ETHERNET)
  B00000000, // not a port
  B00000000, // not a port
  // Bits 6, 7 have not correspondented pins in Arduino Mini Pro / Freeduino 2009
  B00111101, /*     PORTB        
D13 -^    ^- D8    <- pins   */
  B00000000, /*     PORTC 
   ^-A7   ^-A0   <- pins    */
  B00000011  /*     PORTD 
   ^-D7   ^-D0   <- pins    */
#elif defined(ARDUINO_AVR_LEONARDO) || defined (ARDUINO_AVR_MICRO) || defined(ARDUINO_AVR_ROBOT_CONTROL) || defined(ARDUINO_AVR_ROBOT_MOTOR) || defined (ARDUINO_AVR_YUN)
  // check ports settings for your platform
  B00000000, // not a port
  B00000000, // not a port
  B00000000, // PORTB
  B00000000, // PORTC
  B00000000, // PORTD
  B00000000, // PORTE
  B00000000  // PORTF
#elif defined (ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560)
  // check ports settings for your platform
  B00000000, // not a port
  B00000000, // PORTA
  B00000000, // PORTB
  B00000000, // PORTC
  B00000000, // PORTD
  B00000000, // PORTE
  B00000000, // PORTF
  B00000000, // PORTG
  B00000000, // PORTH
  B00000000, // not a port
  B00000000, // PORTJ
  B00000000, // PORTK
  B00000000  // PORTL
#endif
};


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                            COMMAND NAMES SECTION 
*/
// Increase this if add new command 
#define CMD_MAX                                                 0x37

// Add command macro with new sequental number
#define CMD_ZBX_NOPE                                            0x00
#define CMD_ZBX_AGENT_PING                                      0x01
#define CMD_ZBX_AGENT_HOSTNAME                                  0x02
#define CMD_ZBX_AGENT_VERSION                                   0x03
#define CMD_SYS_UPTIME                                          0x04
                                                                
#define CMD_ARDUINO_ANALOGWRITE                                 0x05
#define CMD_ARDUINO_ANALOGREAD                                  0x06
#define CMD_ARDUINO_ANALOGREFERENCE                             0x07
#define CMD_ARDUINO_DELAY                                       0x08
#define CMD_ARDUINO_DIGITALWRITE                                0x09
#define CMD_ARDUINO_DIGITALREAD                                 0x0A
#define CMD_ARDUINO_TONE                                        0x0B
#define CMD_ARDUINO_NOTONE                                      0x0C
#define CMD_ARDUINO_RANDOMSEED                                  0x0D
#define CMD_ARDUINO_RANDOM                                      0x0E

#define CMD_SET_HOSTNAME                                        0x0F
#define CMD_SET_PASSWORD                                        0x10
#define CMD_SET_SYSPROTECT                                      0x11
#define CMD_SET_NETWORK                                         0x12

#define CMD_SYS_PORTWRITE                                       0x13
#define CMD_SYS_SHIFTOUT                                        0x14
#define CMD_SYS_REBOOT                                          0x15

#define CMD_SYS_MCU_NAME                                        0x16
#define CMD_SYS_NET_MODULE                                      0x17

#define CMD_SYS_CMD_COUNT                                       0x18
#define CMD_SYS_CMD_TIMEMAX                                     0x19
#define CMD_SYS_CMD_TIMEMAX_N                                   0x1A

#define CMD_SYS_RAM_FREE                                        0x1B
#define CMD_SYS_RAM_FREEMIN                                     0x1C

#define CMD_SYS_VCC                                             0x1D
#define CMD_SYS_VCCMIN                                          0x1E
#define CMD_SYS_VCCMAX                                          0x1F

#define CMD_EXTINT_COUNT                                        0x20
#define CMD_ENCODER_COUNT                                       0x21

#define CMD_OW_SCAN                                             0x22

#define CMD_I2C_SCAN                                            0x23
#define CMD_I2C_WRITE                                           0x24
#define CMD_I2C_READ                                            0x25
#define CMD_I2C_BITWRITE                                        0x26
#define CMD_I2C_BITREAD                                         0x27

#define CMD_DS18X20_TEMPERATURE                                 0x28

#define CMD_DHT_HUMIDITY                                        0x29
#define CMD_DHT_TEMPERATURE                                     0x2A

#define CMD_BMP_PRESSURE                                        0x2B 
#define CMD_BMP_TEMPERATURE                                     0x2C
#define CMD_BME_HUMIDITY                                        0x2D

#define CMD_BH1750_LIGHT                                        0x2E

#define CMD_MAX7219_WRITE                                       0x2F

#define CMD_PCF8574_LCDPRINT                                    0x30

#define CMD_SHT2X_HUMIDITY                                      0x31
#define CMD_SHT2X_TEMPERATURE                                   0x32

#define CMD_ACS7XX_ZC                                           0x33
#define CMD_ACS7XX_AC                                           0x34
#define CMD_ACS7XX_DC                                           0x35

#define CMD_ULTRASONIC_DISTANCE                                 0x36




// add new command as "const char command_<COMMAND_MACRO> PROGMEM". Only 'const' push string to PROGMEM. Tanx, Arduino.
// command_* values must be in lower case
const char command_CMD_ZBX_NOPE[]                               PROGMEM = "";
const char command_CMD_ZBX_AGENT_PING[]                         PROGMEM = "agent.ping";
const char command_CMD_ZBX_AGENT_HOSTNAME[]                     PROGMEM = "agent.hostname";
const char command_CMD_ZBX_AGENT_VERSION[]                      PROGMEM = "agent.version";
const char command_CMD_SYS_UPTIME[]                             PROGMEM = "sys.uptime";

const char command_CMD_ARDUINO_ANALOGWRITE[]                    PROGMEM = "analogwrite";         
const char command_CMD_ARDUINO_ANALOGREAD[]                     PROGMEM = "analogread";
const char command_CMD_ARDUINO_ANALOGREFERENCE[]                PROGMEM = "analogreference";
const char command_CMD_ARDUINO_DELAY[]                          PROGMEM = "delay";
const char command_CMD_ARDUINO_DIGITALWRITE[]                   PROGMEM = "digitalwrite";
const char command_CMD_ARDUINO_DIGITALREAD[]                    PROGMEM = "digitalread"; 

const char command_CMD_ARDUINO_TONE[]                           PROGMEM = "tone";
const char command_CMD_ARDUINO_NOTONE[]                         PROGMEM = "notone";

const char command_CMD_ARDUINO_RANDOMSEED[]                     PROGMEM = "randomseed";
const char command_CMD_ARDUINO_RANDOM[]                         PROGMEM = "random";

const char command_CMD_SET_HOSTNAME[]                           PROGMEM = "set.hostname";
const char command_CMD_SET_PASSWORD[]                           PROGMEM = "set.password";
const char command_CMD_SET_SYSPROTECT[]                         PROGMEM = "set.sysprotect";
const char command_CMD_SET_NETWORK[]                            PROGMEM = "set.network";

const char command_CMD_SYS_PORTWRITE[]                          PROGMEM = "portwrite";

const char command_CMD_SYS_SHIFTOUT[]                           PROGMEM = "shiftout";

const char command_CMD_SYS_REBOOT[]                             PROGMEM = "reboot";              

const char command_CMD_SYS_MCU_NAME[]                           PROGMEM = "sys.mcu.name";
const char command_CMD_SYS_NET_MODULE[]                         PROGMEM = "sys.net.module";

const char command_CMD_SYS_CMD_COUNT[]                          PROGMEM = "sys.cmd.count";
const char command_CMD_SYS_CMD_TIMEMAX[]                        PROGMEM = "sys.cmd.timemax";
const char command_CMD_SYS_CMD_TIMEMAX_N[]                      PROGMEM = "sys.cmd.timemax.n";

const char command_CMD_SYS_RAM_FREE[]                           PROGMEM = "sys.ram.free";
const char command_CMD_SYS_RAM_FREEMIN[]                        PROGMEM = "sys.ram.freemin";

const char command_CMD_SYS_VCC[]                                PROGMEM = "sys.vcc";
const char command_CMD_SYS_VCCMIN[]                             PROGMEM = "sys.vccmin";
const char command_CMD_SYS_VCCMAX[]                             PROGMEM = "sys.vccmax";

const char command_CMD_EXTINT_COUNT[]                           PROGMEM = "extint.count";
const char command_CMD_ENCODER_COUNT[]                          PROGMEM = "incenc.count";

const char command_CMD_OW_SCAN[]                                PROGMEM = "ow.scan";

const char command_CMD_I2C_SCAN[]                               PROGMEM = "i2c.scan";
const char command_CMD_I2C_WRITE[]                              PROGMEM = "i2c.write";
const char command_CMD_I2C_READ[]                               PROGMEM = "i2c.read";
const char command_CMD_I2C_BITWRITE[]                           PROGMEM = "i2c.bitwrite";
const char command_CMD_I2C_BITREAD[]                            PROGMEM = "i2c.bitread";

const char command_CMD_DS18X20_TEMPERATURE[]                    PROGMEM = "ds18x20.temperature";

const char command_CMD_DHT_HUMIDITY[]                           PROGMEM = "dht.humidity";
const char command_CMD_DHT_TEMPERATURE[]                        PROGMEM = "dht.temperature";

const char command_CMD_BMP_PRESSURE[]                           PROGMEM = "bmp.pressure";
const char command_CMD_BMP_TEMPERATURE[]                        PROGMEM = "bmp.temperature";
const char command_CMD_BME_HUMIDITY[]                           PROGMEM = "bme.humidity";

const char command_CMD_BH1750_LIGHT[]                           PROGMEM = "bh1750.light";

const char command_CMD_MAX7219_WRITE[]                          PROGMEM = "max7219.write";

const char command_CMD_PCF8574_LCDPRINT[]                       PROGMEM = "pcf8574.lcdprint";

const char command_CMD_SHT2X_HUMIDITY[]                         PROGMEM = "sht2x.humidity";
const char command_CMD_SHT2X_TEMPERATURE[]                      PROGMEM = "sht2x.temperature";

const char command_CMD_ACS7XX_ZC[]                              PROGMEM = "acs7xx.zc";
const char command_CMD_ACS7XX_AC[]                              PROGMEM = "acs7xx.ac";
const char command_CMD_ACS7XX_DC[]                              PROGMEM = "acs7xx.dc";

const char command_CMD_ULTRASONIC_DISTANCE[]                    PROGMEM = "ultrasonic.distance";

// do not insert new command to any position without syncing indexes. Tanx, Arduino, for this method of string array pushing to PROGMEM
// ~300 bytes of PROGMEM space can be saved with crazy "#ifdef-#else-#endif" dance
const char* const commands[] PROGMEM = {
  command_CMD_ZBX_NOPE,

  command_CMD_ZBX_AGENT_PING,
  command_CMD_ZBX_AGENT_HOSTNAME,
  command_CMD_ZBX_AGENT_VERSION,
  command_CMD_SYS_UPTIME,

  command_CMD_ARDUINO_ANALOGWRITE,
  command_CMD_ARDUINO_ANALOGREAD,

#ifdef FEATURE_AREF_ENABLE
  command_CMD_ARDUINO_ANALOGREFERENCE,
#else
  command_CMD_ZBX_NOPE,
#endif

  command_CMD_ARDUINO_DELAY,
  command_CMD_ARDUINO_DIGITALWRITE,
  command_CMD_ARDUINO_DIGITALREAD,

#ifdef FEATURE_TONE_ENABLE
  command_CMD_ARDUINO_TONE,
  command_CMD_ARDUINO_NOTONE,
#else
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
#endif

#ifdef FEATURE_RANDOM_ENABLE
  command_CMD_ARDUINO_RANDOMSEED,
  command_CMD_ARDUINO_RANDOM,
#else
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
#endif

#ifdef FEATURE_EEPROM_ENABLE
  command_CMD_SET_HOSTNAME,
  command_CMD_SET_PASSWORD,
  command_CMD_SET_SYSPROTECT,
  command_CMD_SET_NETWORK,
#else
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
#endif

  command_CMD_SYS_PORTWRITE,

#ifdef FEATURE_SHIFTOUT_ENABLE 
  command_CMD_SYS_SHIFTOUT,
#else
  command_CMD_ZBX_NOPE,
#endif

  command_CMD_SYS_REBOOT,

#ifdef FEATURE_DEBUG_COMMANDS_ENABLE
  command_CMD_SYS_MCU_NAME,
  command_CMD_SYS_NET_MODULE,
  command_CMD_SYS_CMD_COUNT,
  command_CMD_SYS_CMD_TIMEMAX,
  command_CMD_SYS_CMD_TIMEMAX_N,
  
  command_CMD_SYS_RAM_FREE,
  command_CMD_SYS_RAM_FREEMIN,
#else 
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
#endif
  
  command_CMD_SYS_VCC,
  command_CMD_SYS_VCCMIN,
  command_CMD_SYS_VCCMAX,

#ifdef FEATURE_EXTERNAL_INTERRUPT_ENABLE
  command_CMD_EXTINT_COUNT,
#else
  command_CMD_ZBX_NOPE,
#endif

#ifdef FEATURE_ENCODER_ENABLE
  command_CMD_ENCODER_COUNT,
#else
  command_CMD_ZBX_NOPE,
#endif

#ifdef FEATURE_OW_ENABLE
  command_CMD_OW_SCAN,
#else
  command_CMD_ZBX_NOPE,
#endif

#ifdef FEATURE_I2C_ENABLE
  command_CMD_I2C_SCAN,
  command_CMD_I2C_WRITE,
  command_CMD_I2C_READ,
  command_CMD_I2C_BITWRITE,
  command_CMD_I2C_BITREAD,
#else
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
#endif

#ifdef FEATURE_DS18X20_ENABLE
  command_CMD_DS18X20_TEMPERATURE,
#else
  command_CMD_ZBX_NOPE,
#endif

#ifdef FEATURE_DHT_ENABLE
  command_CMD_DHT_HUMIDITY,
  command_CMD_DHT_TEMPERATURE,
#else
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
#endif

#ifdef FEATURE_BMP_ENABLE
  command_CMD_BMP_PRESSURE,
  command_CMD_BMP_TEMPERATURE,
#else
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
#endif

#ifdef SUPPORT_BME280_INCLUDE
  command_CMD_BME_HUMIDITY,
#else
  command_CMD_ZBX_NOPE,
#endif

#ifdef FEATURE_BH1750_ENABLE
  command_CMD_BH1750_LIGHT,
#else
  command_CMD_ZBX_NOPE,
#endif

#ifdef FEATURE_MAX7219_ENABLE
  command_CMD_MAX7219_WRITE,
#else
  command_CMD_ZBX_NOPE,
#endif

#ifdef FEATURE_PCF8574_LCD_ENABLE
  command_CMD_PCF8574_LCDPRINT,
#else
  command_CMD_ZBX_NOPE,
#endif
  
#ifdef FEATURE_SHT2X_ENABLE
  command_CMD_SHT2X_HUMIDITY,
  command_CMD_SHT2X_TEMPERATURE,
#else
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
#endif
  
#ifdef FEATURE_ACS7XX_ENABLE
  command_CMD_ACS7XX_ZC,
  command_CMD_ACS7XX_AC,
  command_CMD_ACS7XX_DC,
#else
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
  command_CMD_ZBX_NOPE,
#endif

#ifdef FEATURE_ULTRASONIC_ENABLE
  command_CMD_ULTRASONIC_DISTANCE,
#else
  command_CMD_ZBX_NOPE,
#endif

 
};
/*
/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                           VARIOUS DEFINES SECTION 
*/

#define IDX_METRICS_MAX                                         8
#define IDX_METRICS_FIRST_CRONNED                               3

#define IDX_METRIC_SYS_CMD_COUNT                                0
#define IDX_METRIC_SYS_CMD_TIMEMAX                              1
#define IDX_METRIC_SYS_CMD_TIMEMAX_N                            2

#define IDX_METRIC_SYS_VCC                                      3
#define IDX_METRIC_SYS_VCCMIN                                   4
#define IDX_METRIC_SYS_VCCMAX                                   5
#define IDX_METRIC_SYS_RAM_FREE                                 6
#define IDX_METRIC_SYS_RAM_FREEMIN                              7

// Zabbix v2.x header prefix ('ZBXD\x01')
#define ZBX_HEADER_PREFIX                                       "zbxd\1"
// sizeof() give wrong result -> 6
#define ZBX_HEADER_PREFIX_LENGTH                                5
// Zabbix v2.x header length
#define ZBX_HEADER_LENGTH                                       12

#define SENS_READ_TEMP                                          0x01
#define SENS_READ_HUMD                                          0x02
#define SENS_READ_PRSS                                          0x03
#define SENS_READ_LUX                                           0x04

#define SENS_READ_ZC                                            0x08
#define SENS_READ_AC                                            0x09
#define SENS_READ_DC                                            0x0A

#define SENS_READ_RAW                                           0xFF

#define RESULT_IS_FAIL                                          -0xFFAL
#define RESULT_IS_OK                                            -0xFFBL
#define RESULT_IN_BUFFER                                        -0xFFCL
#define RESULT_IS_PRINTED                                       -0xFFDL

// Error Codes
//#define DEVICE_DISCONNECTED_C         	-127
#define DEVICE_ERROR_CONNECT                                    -127
#define DEVICE_ERROR_ACK_L                                      -128
#define DEVICE_ERROR_ACK_H                                      -129
#define DEVICE_ERROR_CHECKSUM                                   -131
#define DEVICE_ERROR_TIMEOUT                                    -130

/*
ADC channels 

     • Bits 3:0 – MUX[3:0]: Analog Channel Selection Bits
       The value of these bits selects which analog inputs are connected to the ADC. See Table 24-4 for details. If
       these bits are changed during a conversion, the change will not go in effect until this conversion is complete
       (ADIF in ADCSRA is set).

       Table 24-4. Input Channel Selections
       MUX3..0  Single Ended Input
       1110     1.1V (VBG)
       1111     0V (GND)       - noise level measurement possible
*/
#define ANALOG_CHAN_VBG 		                        0x0E // B1110
#define ANALOG_CHAN_GND 		                        0x0F // B1111

#define DBG_PRINT_AS_MAC 		                        0x01
#define DBG_PRINT_AS_IP  		                        0x02

#define ENCODER_STABILIZATION_DELAY                             2000L // 2000 microseconds
#define I2C_NO_REG_SPECIFIED                                    -0x01 //



/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                         PROGRAMM STRUCTURES SECTION 
*/
// Note: netconfig_t size must be no more ___uint8_t___ bytes, because readConfig()'s read cycle use uint8_t counter. 
// Change the index's variable type if bigger size need
typedef struct {
  uint8_t useDHCP;         	 		  // 1 byte
  uint8_t macAddress[6];                          // 6 byte 
  IPAddress ipAddress;     			  // 6 byte (uint8_t[])
  IPAddress ipNetmask;     			  // 6 byte (uint8_t[])
  IPAddress ipGateway;     			  // 6 byte (uint8_t[])
  char hostname[ZBX_AGENT_HOSTNAME_MAXLEN];       // 255 - (1 + 6*4 + 4 + 1) = 225 bytes max
  // #ifdef ... #elif ... #endif does not work with struct operator
  uint32_t password;                              // 4 byte
  uint8_t useProtection;                          // 1 byte
} netconfig_t ;


typedef struct {
  uint32_t count;        			  // 4 byte
  // mode == -1 => Interrupt is not attached
  int8_t mode;                                    // 1 byte 
  int8_t encTerminalAPin;                         // 1 byte 
  int8_t encTerminalBPin;                         // 1 byte 
} extInterrupt_t ;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                         INLINE AND "DEFINE" FUNCTIONS SECTION 
*/

// HEX char to number
#define htod(_hex) ( (_hex >= 'a' && _hex <= 'f') ? (10 + _hex - 'a') : ( (_hex >= '0' && _hex <= '9') ? (_hex - '0') : 0)) 

// Convert dec number to hex char
#define dtoh(_dec) ( (_dec > 9) ? ('A' - 10 + _dec) : ('0' + _dec) )

// if _source have hex prefix - return true
#define haveHexPrefix(_source) ( (_source[0] == '0' && _source[1] == 'x') )


#endif

