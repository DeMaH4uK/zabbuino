// My Freeduino is not listed, but is analogue to ARDUINO_AVR_DUEMILANOVE
#define ARDUINO_AVR_DUEMILANOVE
// Just for compilation with various default network configs
//#define USE_NETWORK_192_168_0_1

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                             !!! WizNet W5xxx users !!!

    1. Comment #include <UIPEthernet.h>
    2. Uncomment #include <Ethernet.h> and <SPI.h> headers
*/
#include <Ethernet.h>
#include <SPI.h>

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                                !!! ENC28J60 users !!!

    0. Please try to use https://github.com/ntruchsess/arduino_uip/tree/fix_errata12 brahch of UIPEthernet if your ENC28J60 freeze or loose connection.
    
    1. Comment #include <Ethernet.h> and <SPI.h> headers
    2. Uncomment #include <UIPEthernet.h> 
    
    Tested on UIPEthernet v1.09
    
    When UIPEthernet's fix_errata12 brahch did not help to add stability, you can buy W5100 shield.
    Also u can try uncomment USE_DIRTY_HACK_AND_REBOOT_ENC28J60_IF_ITS_SEEMS_FREEZE declaration (then ENC will be periodically re-intit if EIR.TXERIF and EIR.RXERIF is 1), 
    but you eed to do one change in UIPEthernet\utility\Enc28J60Network.h :
         private:
            ...    
            static uint8_t readReg(uint8_t address);  // << move its to __public__ section
            ...
             
         public: 
             ...

*/
//#include <UIPEthernet.h>
//#define USE_DIRTY_HACK_AND_REBOOT_ENC28J60_IF_ITS_SEEMS_FREEZE


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                                 PROGRAMM FEATURES SECTION
                                                                        MOVED  TO
                                                                        zabuino.h
                   
                Please refer to the zabbuino.h file for enabling or disabling Zabbuino's features  and tuning (like set State LED pin, network addresses, agent hostname and so)

        if connected sensors seems not work - first check setting in port_protect[], port_mode[], port_pullup[] arrays in I/O PORTS SETTING SECTION

*/


#include "zabbuino.h"

// Wire lib for I2C sensors
#include <Wire.h>
// OneWire lib for Dallas sensors
#include <OneWire.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
// used by interrupts-related macroses
#include <wiring_private.h>

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                                 GLOBAL VARIABLES SECTION
*/

netconfig_t netConfig;
#if defined(FEATURE_EXTERNAL_INTERRUPT_ENABLE) || defined(FEATURE_ENCODER_ENABLE)
// need to #include <wiring_private.h> for compilation
volatile extInterrupt_t extInterrupt[EXTERNAL_NUM_INTERRUPTS];
#endif

#ifdef FEATURE_IR_ENABLE
uint8_t irPWMPin;
#endif

EthernetServer ethServer(10050);
EthernetClient ethClient;

char cBuffer[BUFFER_SIZE+1]; // +1 for trailing \0
int16_t argOffset[ARGS_MAX];
int32_t sysMetrics[IDX_METRICS_MAX];
// skipMetricGathering used in interrupt's subroutine - must be `volatile` 
volatile uint8_t skipMetricGathering = false;

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                                      STARTUP SECTION
*/

void setup() {
  uint8_t i;

#ifdef ADVANCED_BLINKING
  // blink on start
  blinkMore(6, 50, 500);
#endif

  // Init metrics
  sysMetrics[IDX_METRIC_SYS_VCCMIN] = sysMetrics[IDX_METRIC_SYS_VCCMAX] = MeasureVoltage(ANALOG_CHAN_VBG);
  sysMetrics[IDX_METRIC_SYS_RAM_FREE] = sysMetrics[IDX_METRIC_SYS_RAM_FREEMIN] = (int32_t) ramFree();
  sysMetrics[IDX_METRIC_SYS_CMD_COUNT] = sysMetrics[IDX_METRIC_SYS_CMD_TIMEMAX] = 0;
  
#ifdef FEATURE_DEBUG_TO_SERIAL
  Serial.begin(9600);
  while (!Serial);             // Leonardo: wait for serial monitor
#endif

#ifdef FEATURE_EEPROM_ENABLE

/* -=-=-=-=-=-=-=-=-=-=-=-
    FACTORY RESET BLOCK
   -=-=-=-=-=-=-=-=-=-=-=- */

  // Set mode of PIN_FACTORY_RESET and turn on internal pull resistor
  pinMode(PIN_STATE_LED, OUTPUT);
  digitalWrite(PIN_STATE_LED, LOW);
  pinMode(PIN_FACTORY_RESET, INPUT_PULLUP);
  // Check for PIN_FACTORY_RESET shorting to ground?
  // (when pulled INPUT pin shorted to GND - digitalRead() return LOW)
  if (LOW == digitalRead(PIN_FACTORY_RESET)){
    // Fire up state LED
    digitalWrite(PIN_STATE_LED, HIGH);
    // Wait some msecs
    delay(HOLD_TIME_TO_FACTORY_RESET);
    // PIN_FACTORY_RESET still shorted?
    if (LOW == digitalRead(PIN_FACTORY_RESET)){
       setDefaults(netConfig);
       saveConfig(&netConfig);
       // Blink fast while PIN_FACTORY_RESET shorted to GND
       while (LOW == digitalRead(PIN_FACTORY_RESET)) {
          digitalWrite(PIN_STATE_LED, millis() % 100 < 50);
      }
    }
    digitalWrite(PIN_STATE_LED, LOW);
  } // if (LOW == digitalRead(PIN_FACTORY_RESET))

/* -=-=-=-=-=-=-=-=-=-=-=-
    CONFIGURATION LOAD BLOCK
   -=-=-=-=-=-=-=-=-=-=-=- */
#ifdef FEATURE_DEBUG_TO_SERIAL
  SerialPrintln_P(PSTR("Load configuration from EEPROM"));
#endif
  // Try to load configuration from EEPROM
  if (false == loadConfig(&netConfig)) {
     // bad CRC detected, use default values for this run
     setDefaults(netConfig);
     saveConfig(&netConfig);
  }
#else // FEATURE_EEPROM_ENABLE
#ifdef FEATURE_DEBUG_TO_SERIAL
     SerialPrintln_P(PSTR("Use default network settings"));
#endif
     // Use hardcoded values if EEPROM feature disabled
     setDefaults(netConfig);
#endif // FEATURE_EEPROM_ENABLE

#ifdef FEATURE_NET_DHCP_FORCE
     netConfig.useDHCP = true;
#endif


/* -=-=-=-=-=-=-=-=-=-=-=-
    NETWORK START BLOCK
   -=-=-=-=-=-=-=-=-=-=-=- */
#ifdef FEATURE_NET_DHCP_ENABLE
  // User want to use DHCP with Zabbuino?
  if (true == netConfig.useDHCP) {
#ifdef FEATURE_DEBUG_TO_SERIAL
     SerialPrintln_P(PSTR("Obtaining address from DHCP..."));
#endif
       // Try to ask DHCP server
      if (0 == Ethernet.begin(netConfig.macAddress)) {
#ifdef FEATURE_DEBUG_TO_SERIAL
         SerialPrintln_P(PSTR("No successfully"));
#endif
         // No offer recieved - switch off DHCP feature for that session
         netConfig.useDHCP = false;
      }
  }
#else // FEATURE_NET_DHCP_ENABLE
  netConfig.useDHCP=false;
#endif // FEATURE_NET_DHCP_ENABLE

  // No DHCP offer recieved or no DHCP need - start with stored/default IP config
  if (false == netConfig.useDHCP) {
#ifdef FEATURE_DEBUG_TO_SERIAL
     SerialPrintln_P(PSTR("Use static IP"));
#endif
     // That overloaded .begin() function return nothing
     Ethernet.begin(netConfig.macAddress, netConfig.ipAddress, netConfig.ipNetmask, netConfig.ipGateway);
  }
  
#ifdef FEATURE_DEBUG_TO_SERIAL
  SerialPrintln_P(PSTR("Begin internetworking"));
  SerialPrint_P(PSTR("MAC     : ")); printArray(netConfig.macAddress, sizeof(netConfig.macAddress), DBG_PRINT_AS_MAC);
  SerialPrint_P(PSTR("Hostname: ")); Serial.println(netConfig.hostname);
  SerialPrint_P(PSTR("IP      : ")); Serial.println(Ethernet.localIP());
  SerialPrint_P(PSTR("Subnet  : ")); Serial.println(Ethernet.subnetMask());
  SerialPrint_P(PSTR("Gateway : ")); Serial.println(Ethernet.gatewayIP());
  SerialPrint_P(PSTR("Password: ")); Serial.println(netConfig.password, DEC);
  // This codeblock is compiled if UIPethernet.h is included
#ifdef UIPETHERNET_H
  SerialPrint_P(PSTR("ENC28J60: rev ")); Serial.println(Enc28J60.getrev());
#endif

  //netConfig.ipGateway.printTo(Serial);
#endif

  // Start listen sockets
  ethServer.begin();

/* -=-=-=-=-=-=-=-=-=-=-=-
    OTHER STUFF INIT BLOCK
   -=-=-=-=-=-=-=-=-=-=-=- */

  // I/O ports initialization. Refer to "I/O PORTS SETTING SECTION" in zabbuino.h
  for (i = 0; i < PORTS_NUM; i++) { 
    setPortMode(i, port_mode[i], port_pullup[i]);
  }

#if defined(FEATURE_EXTERNAL_INTERRUPT_ENABLE) || defined(FEATURE_ENCODER_ENABLE)
  // Init external interrupts info structure
  for (i = 0; i < EXTERNAL_NUM_INTERRUPTS; i++) { 
    // -1 - interrupt is detached
    // just use NOT_AN_INTERRUPT = -1 macro from Arduino.h
    extInterrupt[i].mode = NOT_AN_INTERRUPT;
  }
#endif

  // Uncomment to force protect (enable even useProtection is false) your system from illegal access for change runtime settings and reboots 
#ifdef FEATURE_PASSWORD_PROTECTION_FORCE
  netConfig.useProtection = true;
#endif

#ifdef FEATURE_WATCHDOG_ENABLE
  // Watchdog activation
  wdt_enable(WTD_TIMEOUT);
#endif

#ifdef GATHER_METRIC_USING_TIMER_INTERRUPT
   // need to analyze return code?
   timerOneInit(SYS_METRIC_RENEW_PERIOD);
#endif

#if defined(FEATURE_I2C_ENABLE) || defined(FEATURE_BMP_ENABLE) || defined(FEATURE_BH1750_ENABLE) || defined (FEATURE_PCF8574_LCD_ENABLE) || defined (FEATURE_SHT2X_ENABLE)
Wire.begin();
#endif

#ifdef ADVANCED_BLINKING
  // blink on init end
  blinkMore(2, 1000, 1000);
#endif

}


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                                      RUN SECTION
*/
void loop() {
  uint32_t nowTime, processStartTime, processEndTime;
  uint32_t prevDHCPRenewTime, prevENCReInitTime, prevNetProblemTime, prevSysMetricGatherTime;
  uint8_t errorCode, blinkType = (uint8_t) BLINK_NOPE;
    
  // Correcting timestamps

  prevDHCPRenewTime = prevENCReInitTime = prevNetProblemTime = prevSysMetricGatherTime = millis();

  // if no exist while() here - netProblemTime must be global or static - its will be 0 every loop() and time-related cases will be processeed abnormally
  // ...and while save some cpu ticks because do not call everytime from "hidden main()" subroutine, and do not init var, and so.
  while (true) { 
    nowTime = millis();

#ifndef GATHER_METRIC_USING_TIMER_INTERRUPT
    // Gather internal metrics periodically
    if (SYS_METRIC_RENEW_PERIOD <= (uint32_t) (nowTime - prevSysMetricGatherTime)) { gatherMetrics(); prevSysMetricGatherTime = nowTime; }
#endif 

#ifdef FEATURE_NET_DHCP_ENABLE
    // DHCP used in this session and time to renew lease?
      // how many overhead give Ethernet.maintain() ?
//    if (true == netConfig.useDHCP && (NET_DHCP_RENEW_PERIOD <= (uint32_t) (nowTime - prevDHCPRenewTime))) {
       // Ethernet library's manual say that Ethernet.maintain() can be called every loop for DHCP renew, but i won't do this so often
       errorCode = Ethernet.maintain();
       // Renew procedure finished with success
       if (DHCP_CHECK_RENEW_OK == errorCode || DHCP_CHECK_REBIND_OK  == errorCode) { 
          // No alarm blink  need, network activity registred, renewal period restarted
          blinkType = (uint8_t) BLINK_NOPE;
//          prevDHCPRenewTime = prevNetProblemTime = nowTime;
       } else {
          // Got some errors - blink with "DHCP problem message"
          blinkType = (uint8_t) BLINK_DHCP_PROBLEM;
#ifdef FEATURE_DEBUG_TO_SERIAL
//            SerialPrintln_P(PSTR("DHCP renew problem occured"));
#endif 
       }
//    }
#endif // FEATURE_NET_DHCP_ENABLE

    // No DHCP problem found but no data recieved or network activity for a long time
    if (BLINK_NOPE == blinkType && (NET_IDLE_TIMEOUT <= (uint32_t) (nowTime - prevNetProblemTime))) { 
#ifdef FEATURE_DEBUG_TO_SERIAL
//            SerialPrintln_P(PSTR("No data recieved for a long time"));
#endif 
       blinkType =(uint8_t) BLINK_NET_PROBLEM; 
    }

#ifdef USE_DIRTY_HACK_AND_REBOOT_ENC28J60_IF_ITS_SEEMS_FREEZE
    // Time to reinit ENC28J60?
    if (NET_ENC28J60_REINIT_PERIOD <= (uint32_t) (nowTime - prevENCReInitTime)) {
       // if EIR.TXERIF or EIR.RXERIF is set - ENC28J60 detect error, re-init module 
       if (Enc28J60.readReg(EIR) & (EIR_TXERIF | EIR_RXERIF)) {
#ifdef FEATURE_DEBUG_TO_SERIAL
          SerialPrintln_P(PSTR("ENC28J60 reinit"));
#endif
          Enc28J60.init(netConfig.macAddress); 
          delay(NET_STABILIZATION_DELAY);
       } 
         prevENCReInitTime = nowTime;

    }
#endif // USE_DIRTY_HACK_AND_REBOOT_ENC28J60_IF_ITS_SEEMS_FREEZE

    // No errors in the loop?
    if (BLINK_NOPE == blinkType) {
       // Switch off state led
       digitalWrite(PIN_STATE_LED, LOW);
    } else {
      // Error caused
#ifdef ON_ALARM_STATE_BLINK
      // Do LED blink...
      digitalWrite(PIN_STATE_LED, nowTime % 1000 < blinkType);
#else
      // ...or just fired up
      digitalWrite(PIN_STATE_LED, HIGH);
#endif
    }

    // Test state of active session: client still connected or unread data is exist in buffer?
    if (ethClient.connected()) {
       // A lot of chars wait for reading
       if (ethClient.available()) {
          // Do not need next char to analyze - EOL detected or there no room in buffer or max number or args parsed...   
          if (false == analyzeStream(ethClient.read())) {
             /*****  processing command *****/
             // Destroy unused client's data 
             ethClient.flush(); 
             // Fire up State led, than will be turned off on next loop
             digitalWrite(PIN_STATE_LED, HIGH);
             //
             // may be need test for client.connected()? 
             processStartTime = millis();
             uint8_t cmdIdx = executeCommand();
             processEndTime = millis();
             // use processEndTime as processDurationTime
             processEndTime = (processStartTime <= processEndTime) ? (processEndTime - processStartTime) : (4294967295UL - processStartTime + processEndTime);
             if (sysMetrics[IDX_METRIC_SYS_CMD_TIMEMAX] < processEndTime){
                sysMetrics[IDX_METRIC_SYS_CMD_TIMEMAX] = processEndTime;
                sysMetrics[IDX_METRIC_SYS_CMD_TIMEMAX_N] = cmdIdx;
             }
             // Wait some time to finishing answer send
             delay(NET_STABILIZATION_DELAY);
             // close connection           
             ethClient.stop(); 
          }
          // Restart network activity control cycle
          prevENCReInitTime = prevNetProblemTime = millis();
          blinkType = (uint8_t) BLINK_NOPE;
       }
    } else {
       // Active session is not exist. Try to take new for processing.
       ethClient = ethServer.available();
    }
#ifdef FEATURE_WATCHDOG_ENABLE
    // reset watchdog every loop
    wdt_reset();
#endif
  }
}


/* ****************************************************************************************************************************
*
*  Stream analyzing subroutine
*  Detect Zabbix packets, on-fly spit incoming stream to command & arguments
*
**************************************************************************************************************************** */
uint8_t analyzeStream(char charFromClient) {
  uint8_t static needSkipZabbix2Header, argIndex;
  uint16_t static bufferWritePosition;

  // If there is not room in buffer - simulate EOL recieving
  if (BUFFER_SIZE <= bufferWritePosition ) { charFromClient = '\n'; 
//     Serial.println("End of buffer reached, stop analyzing");
  }
  
  // Put next char to buffer
//  Serial.print("[");  Serial.print(bufferWritePosition);  Serial.print("] ");  
//   if (charFromClient > 32) { Serial.print(charFromClient); } else {Serial.print(" ");}
//  Serial.print(" => "); Serial.print(charFromClient, HEX); Serial.print(" = tolower => "); 
  cBuffer[bufferWritePosition] = tolower(charFromClient); 
//  if (cBuffer[bufferWritePosition] > 32) { Serial.print(cBuffer[bufferWritePosition]); } else {Serial.print(" ");}
//  Serial.print(" => "); Serial.println(cBuffer[bufferWritePosition], HEX);
  
  // When ZBX_HEADER_PREFIX_LENGTH chars is saved to buffer - test its for Zabbix2 protocol header prefix ("ZBXD\01") presence
  if (ZBX_HEADER_PREFIX_LENGTH == bufferWritePosition) {
     if (0 == memcmp(&cBuffer, ZBX_HEADER_PREFIX, ZBX_HEADER_PREFIX_LENGTH)) {
        // If packet have prefix - set 'skip whole header' flag
        needSkipZabbix2Header = true;
//        Serial.println("Header detected, skipping it");
     }
  }

  // When ZBX_HEADER_LENGTH chars is saved to buffer - ckeck 'skip whole header' flag
  if (ZBX_HEADER_LENGTH == bufferWritePosition && needSkipZabbix2Header) {
     // If is setted - just begin write new data from begin of buffer. It's operation 'drop' Zabbix2 header
     bufferWritePosition = 0;
     needSkipZabbix2Header = false;
     // Return 'Need next char' and save a lot cpu time 
 //    Serial.println("Header skipped");
     return true;
  }

  // Process all chars if its not from header data
  if (!needSkipZabbix2Header) {
     switch (charFromClient) {
        case ']':
        case 0x20:
          // Space or final square bracket found. Do nothing and next char will be written to same position. 
          // Return 'Need next char'
//          Serial.println("Skip ' ' or ']'");
          return true;
        case '[':
        case ',':
//          Serial.println("Delimiter or separator found, processing");
          // Delimiter or separator found. Push begin of next argument (bufferWritePosition+1) on buffer to arguments offset array. 
          argOffset[argIndex] = bufferWritePosition+1; argIndex++; 
          // Make current buffer segment like C-string
          cBuffer[bufferWritePosition] = '\0'; 
          break;
        case '\n':
//Serial.println();  
//           Serial.println("EOL detected");
          // EOL detected
          // Save last argIndex that pointed to <null> item. All unused argOffset[] items must be pointed to this <null> item too.
          cBuffer[bufferWritePosition] = '\0'; 
          while (ARGS_MAX > argIndex) { argOffset[argIndex++] = bufferWritePosition;}
          // increase argIndex++ to pass (ARGS_MAX < argIndex) condition 
          argIndex++; break;
      }
      // EOL reached or there is not room to store args. Stop stream analyzing and do command executing
      if (ARGS_MAX < argIndex) {
         // clear vars for next round
         bufferWritePosition = argIndex = 0;
         needSkipZabbix2Header = false;
         // Return 'Do not need next char'
         return false;
       }             
  }
  // 
  bufferWritePosition++;
  // Return 'Need next char' and save a lot cpu time 
  return true;
}

/* ****************************************************************************************************************************
*
*  Command execution subroutine
*  
**************************************************************************************************************************** */
uint8_t executeCommand()
{
  uint8_t AccessGranted, i;
  int32_t result = RESULT_IS_FAIL;
  // duration  in tone[] - ulong
  uint32_t arg[ARGS_MAX];
  int16_t cmdIdx = -1;
  
  sysMetrics[IDX_METRIC_SYS_CMD_COUNT]++;

  for (i = 0; i < CMD_MAX; i++)
  {
//    Serial.print(i, HEX); Serial.print(": ");
//    SerialPrintln_P((char*)pgm_read_word(&(commands[i])));
    if (0 == strcmp_P(cBuffer, (char*)pgm_read_word(&(commands[i])))) {cmdIdx = i; break;}
  }

#ifdef FEATURE_DEBUG_TO_SERIAL
  SerialPrint_P(PSTR("Execute command #")); Serial.print(cmdIdx, HEX); SerialPrint_P(PSTR(" =>")); Serial.println(cBuffer);
#endif 
  
  // batch convert args to number values
  for (i = 0; i < ARGS_MAX; i++)
  {
     arg[i] = ('\0' == cBuffer[argOffset[i]]) ? 0 : strtoul(&cBuffer[argOffset[i]], NULL,0);

#ifdef FEATURE_DEBUG_TO_SERIAL
     SerialPrint_P(PSTR("arg[")); Serial.print(i); SerialPrint_P(PSTR("] => \"")); 
     if ('\0' == cBuffer[argOffset[i]]) {
        SerialPrint_P(PSTR("<null>")); 
     } else {
        Serial.print(&cBuffer[argOffset[i]]); 
     }
     SerialPrint_P(PSTR("\" => ")); Serial.print(arg[i]);
     SerialPrint_P(PSTR(", offset =")); Serial.println(argOffset[i]);
#endif 
  }

  
  // Check rights for password protected commands
  AccessGranted = (!netConfig.useProtection || arg[0] == netConfig.password); 

  uint8_t i2CAddress, i2COption;
  uint8_t i2CValue[4];
  int16_t i2CRegister;

  i2CAddress = (uint8_t) arg[2];
  i2CRegister = (int16_t) arg[3];
  // i2COption can be length, bitNumber or data
  i2COption = (uint8_t) arg[4];

 
   switch (cmdIdx) {
//  case  CMD_ZBX_NOPE: 
//        break;
    case CMD_ZBX_AGENT_PING:
      /*/
      //   agent.ping 
      /=*/
      result = RESULT_IS_OK;
      break;

    case CMD_ZBX_AGENT_HOSTNAME:
      /*/
      /=/   agent.hostname
      /*/
      strcpy(cBuffer, netConfig.hostname);
      result = RESULT_IN_BUFFER;
      break;
         
    case CMD_ZBX_AGENT_VERSION:
      /*/
      /=/  agent.version
      /*/
      strcpy_P(cBuffer, PSTR(ZBX_AGENT_VERISON));
      result = RESULT_IN_BUFFER;
      break;

    case CMD_SYS_UPTIME:
      /*/
      /=/  sys.uptime
      /*/
      result = millis() / 1000;
      break;
   


    case CMD_ARDUINO_ANALOGWRITE:
      /*/
      /=/  analogWrite[pin, value]
      /*/
      if (isSafePin(arg[0])) {
         analogWrite(arg[0], arg[1]);
         result = RESULT_IS_OK;
      }
      break;
      
    case CMD_ARDUINO_ANALOGREAD:
      /*/
      /=/  analogRead[pin, analogReferenceSource]
      /*/
#ifdef FEATURE_AREF_ENABLE
      // change source of the reference voltage if its given
      if ('\0' != cBuffer[argOffset[1]]) {
         analogReference(arg[1]);
         delayMicroseconds(2000);
      }
#endif
      // Do not disturb processes by internal routines 
      skipMetricGathering = true;
      result = (int32_t) analogRead(arg[0]);
      skipMetricGathering = false;
      break;      
  
#ifdef FEATURE_AREF_ENABLE
    case CMD_ARDUINO_ANALOGREFERENCE:
      /*/
      /=/  analogReference[source]
      /*/
      analogReference(arg[0]);
      result = RESULT_IS_OK;
      break;
#endif
  
    
    case CMD_ARDUINO_DELAY:
      /*/
      /=/  delay[time]
      /*/
      delay(arg[0]);
      result = RESULT_IS_OK;
      break;

    case CMD_ARDUINO_DIGITALWRITE:
      /*/
      /=/  digitalWrite[pin, value]
      /*/
      if (isSafePin(arg[0])) {
         digitalWrite(arg[0], arg[1]);
         result = RESULT_IS_OK;
      }
      break;
      
    case CMD_ARDUINO_DIGITALREAD:
      /*/
      /=/  digitalRead[pin]
      /*/
      result = (int32_t) digitalRead(arg[0]);
      break;


#ifdef FEATURE_TONE_ENABLE
    case CMD_ARDUINO_TONE:
      /*/
      /=/  tone[pin, frequency, duration]
      /*/
      if (isSafePin(arg[0])) {
        if ('\0' != cBuffer[argOffset[2]]) {
           tone(arg[0], arg[1], arg[2]);
         } else {
           tone(arg[0], arg[1]);
         }
        result = RESULT_IS_OK;
      }
      break;
  
    case CMD_ARDUINO_NOTONE:
      /*/
      /*/
      if (isSafePin(arg[0])) {
        noTone(arg[0]);
        result = RESULT_IS_OK;
      }
      break;
  
#endif

#ifdef FEATURE_RANDOM_ENABLE
    case CMD_ARDUINO_RANDOMSEED:
      /*/
      /=/  randomSeed[value]
      /*/
      randomSeed((0 == arg[0]) ? (int32_t) millis() : arg[0]);
      result = RESULT_IS_OK;
      break;
   
    case CMD_ARDUINO_RANDOM:
      /*/
      /=/  random[min, max]
      /*/
      //  !! random return long
      result = ('\0' == cBuffer[argOffset[1]]) ? (int32_t) random(arg[0]) : (int32_t) random(arg[0], arg[1]);
      break;
#endif // FEATURE_RANDOM_ENABLE


#ifdef FEATURE_EEPROM_ENABLE
// TODO: remove on release
#ifdef FEATURE_EEPROM_SET_COMMANDS_ENABLE
    case CMD_SET_HOSTNAME:
      /*/
      /=/  set.hostname[password, hostname]
      /*/
      if (AccessGranted) {
         // need check for arg existsience?
         // cBuffer[argOffset[1]] != \0 if argument #2 given
         if (cBuffer[argOffset[1]]) {
            sethostname(netConfig.hostname, &cBuffer[argOffset[1]]);
            // strncpy(netConfig.hostname, &cBuffer[argOffset[1]], ZBX_AGENT_HOSTNAME_MAXLEN-1);
            // netConfig.hostname[ZBX_AGENT_HOSTNAME_MAXLEN]='\0';
            //        Serial.println(netConfig.hostname);
            saveConfig(&netConfig);
            result = RESULT_IS_OK;
         }
      }
      break;
  
    case CMD_SET_PASSWORD:
      /*/
      /=/  set.password[oldPassword, newPassword]
      /*/
      if (AccessGranted) {
         if (cBuffer[argOffset[1]]) {
            // take new password from argument #2
            netConfig.password = arg[1];
            saveConfig(&netConfig);
            result = RESULT_IS_OK;
         }
      }
      break;
  
    case CMD_SET_SYSPROTECT:
      /*/
      /=/  set.sysprotect[password, protection]
      /*/
      if (AccessGranted) {
         if (cBuffer[argOffset[1]]) {
            // take new password from argument #2
            netConfig.useProtection = (1 == arg[1]) ? true : false;
            saveConfig(&netConfig);
            result = RESULT_IS_OK;
         }
      }
      break;
   
    case CMD_SET_NETWORK:
      /*/
      /=/  set.network[password, useDHCP, macAddress, ipAddress, ipNetmask, ipGateway]
      /*/
      if (AccessGranted) {
         uint8_t ip[4], mac[6], success = 1;
         // useDHCP flag coming from first argument and must be numeric (boolean) - 1 or 0, 
         // arg[0] data contain in cBuffer[argOffset[1]] placed from argOffset[0]
         netConfig.useDHCP = (uint8_t) arg[1];
         // ip, netmask and gateway have one structure - 4 byte
         // take 6 bytes from second argument of command and use as new MAC-address
         // if convertation is failed (return false) succes variable must be falsed too via logic & operator
         success &= hstoba((uint8_t*) mac, &cBuffer[argOffset[2]], sizeof(netConfig.macAddress));
         memcpy(&netConfig.macAddress, &mac, sizeof(netConfig.macAddress));
      
         // use 4 bytes from third argument of command as new IP-address. sizeof(IPAddress) returns 6 instead 4
         success &= hstoba((uint8_t*) &ip, &cBuffer[argOffset[3]], 4);
         netConfig.ipAddress = IPAddress(ip);
  
         // take 4 bytes from third argument of command an use as new IP Netmask
         success &= hstoba((uint8_t*) &ip, &cBuffer[argOffset[4]], 4);
         netConfig.ipNetmask = IPAddress(ip);
  
         // convert 4 bytes from fourth argument to default gateway
         success &= hstoba((uint8_t*) &ip, &cBuffer[argOffset[5]], 4);
         netConfig.ipGateway = IPAddress(ip);
  
         // netConfig.ipGateway.printTo(Serial);
         // Save config to EEProm if success
         if (success) {
            saveConfig(&netConfig);
            result = RESULT_IS_OK;
         }
       }
       break;
#endif // FEATURE_EEPROM_ENABLE
#endif // 

    case CMD_SYS_PORTWRITE:
      /*/
      /=/  portWrite[port, value]
      /*/
      if (PORTS_NUM < (arg[0] - 96)) {
         portWrite((byte) arg[0] - 96, arg[1]);
         result = RESULT_IS_OK;
      }
      break;
  
#ifdef FEATURE_SHIFTOUT_ENABLE
    case CMD_SYS_SHIFTOUT:
      /*/
      /=/  shiftOut[dataPin, clockPin, latchPin, bitOrder, value]
      /*/
      // i used as latchPinDefined
      i = ('\0' != arg[2]) && isSafePin(arg[2]);   // << корректный способ проверки или нет?  
      if (isSafePin(arg[0]) &&  isSafePin(arg[1])) {
         if (i) { digitalWrite(arg[2], LOW); }
         advShiftOut(arg[0], arg[1], arg[3], &cBuffer[argOffset[4]]);
         if (i) { digitalWrite(arg[2], HIGH);}
         result = RESULT_IS_OK;
      }
      break;
#endif

    case CMD_SYS_REBOOT:
      /*/
      /=/  reboot[password]
      /*/
      if (AccessGranted) {
         ethClient.println("1");
         // hang-up if no delay
         delay(NET_STABILIZATION_DELAY);
         ethClient.stop();
#ifdef FEATURE_WATCHDOG_ENABLE
         // Watchdog deactivation
         wdt_disable();
#endif
         asm volatile ("jmp 0");  
      }
      break;



#ifdef FEATURE_DEBUG_COMMANDS_ENABLE
    case CMD_SYS_MCU_NAME:
      /*/
      /=/  sys.mcu.name
      /*/
      strcpy_P(cBuffer, PSTR(_AVR_CPU_NAME_));
      result = RESULT_IN_BUFFER;
      break;
   
    case CMD_SYS_NET_MODULE:
      /*/
      /=/  sys.net.module
      /*/
      strcpy_P(cBuffer, PSTR(NET_MODULE_NAME));
      result = RESULT_IN_BUFFER;
      break;

    case CMD_SYS_CMD_COUNT:
      /*/
      /=/  sys.cmd.count
      /*/
      if (arg[0]) { sysMetrics[IDX_METRIC_SYS_CMD_COUNT] = 0; } 
      result = sysMetrics[IDX_METRIC_SYS_CMD_COUNT];
      break;

    case CMD_SYS_CMD_TIMEMAX:
      /*/
      /=/  sys.cmd.timemax[resetCounter]
      /*/
      if (cBuffer[argOffset[0]]) { sysMetrics[IDX_METRIC_SYS_CMD_TIMEMAX] = 0; sysMetrics[IDX_METRIC_SYS_CMD_TIMEMAX_N] = 0; } 
      result = sysMetrics[IDX_METRIC_SYS_CMD_TIMEMAX];
      break;

    case CMD_SYS_CMD_TIMEMAX_N:
      /*/
      /=/  sys.cmd.timemax.n
      /*/
      ethClient.println(sysMetrics[IDX_METRIC_SYS_CMD_TIMEMAX_N], HEX);
      result = RESULT_IS_PRINTED;
      break;
   
    case CMD_SYS_RAM_FREE:
      /*/
      /=/  sys.ram.free
      /*/
      result = sysMetrics[IDX_METRIC_SYS_RAM_FREE];
      break;

    case CMD_SYS_RAM_FREEMIN:
      /*/
      /=/  sys.ram.freemin
      /*/
      result = sysMetrics[IDX_METRIC_SYS_RAM_FREEMIN];
      break;

#endif


    case CMD_SYS_VCC:
      /*/
      /=/ sys.vcc
      /*/
      // Take VCC
      result = MeasureVoltage(ANALOG_CHAN_VBG);
      // VCC may be bigger than max or smaller than min. 
      // To avoid wrong results and graphs in monitoring system - correct min/max metrics
      correctVCCMetrics(result);
      break;
  
    case CMD_SYS_VCCMIN:
      /*/
      /=/ sys.vccMin
      /*/
      result = sysMetrics[IDX_METRIC_SYS_VCCMIN];
      break;
  
    case CMD_SYS_VCCMAX:
      /*/
      /=/ sys.vccMax
      /*/
      result = sysMetrics[IDX_METRIC_SYS_VCCMAX];
      break;
  

#ifdef FEATURE_EXTERNAL_INTERRUPT_ENABLE
    case CMD_EXTINT_COUNT:
      /*/
      /=/  extInt.count[intPin, intNumber, mode]
      /*/
      // TODO: maybe need to rework code block
      if (isSafePin(arg[0])) {
         int8_t interruptNumber=digitalPinToInterrupt(arg[0]);
         voidFuncPtr interruptHandler;
         // Interrupt number and mode is correct?
         if ((EXTERNAL_NUM_INTERRUPTS > interruptNumber) && (RISING >= arg[2])) {
            // Interrupt mode is changed
            //Serial.println("[1] Interrupt number and mode is correct"); 
            //Serial.print("[1*] Old interrupt mode is: ");  Serial.println(extInterrupt[interruptNumber].mode); 
            // just use NOT_AN_INTERRUPT = -1 macro from Arduino.h
            if (extInterrupt[interruptNumber].mode != arg[2] && NOT_AN_INTERRUPT != extInterrupt[interruptNumber].mode) {
               //Serial.println("[2] Interrupt mode is changed, detach"); 
               detachInterrupt(arg[1]);
               extInterrupt[interruptNumber].mode = -1;
            } 

           // Interrupt not attached?
           if (NOT_AN_INTERRUPT == extInterrupt[interruptNumber].mode) {
              extInterrupt[interruptNumber].mode = arg[2];
              switch (interruptNumber) {
// Basic configuration => EXTERNAL_NUM_INTERRUPTS == 3
                case INT0:
                  interruptHandler = handleINT0;
                  break;
                case INT1:
                  interruptHandler = handleINT1;
                  break;
// AVR_ATmega1284, AVR_ATmega1284P, AVR_ATmega644, AVR_ATmega644A, AVR_ATmega644P, AVR_ATmega644PA => EXTERNAL_NUM_INTERRUPTS == 3
#if (EXTERNAL_NUM_INTERRUPTS > 2)
                case INT2:
                  interruptHandler = handleINT2;
                  break;
#endif // EXTERNAL_NUM_INTERRUPTS > 2
// AVR_ATmega32U4 => EXTERNAL_NUM_INTERRUPTS == 5
#if (EXTERNAL_NUM_INTERRUPTS > 3)
                case INT3:
                  interruptHandler = handleINT3;
                  break;
                case INT4:
                  interruptHandler = handleINT4;
                  break;
#endif // EXTERNAL_NUM_INTERRUPTS > 3
// AVR_ATmega1280, AVR_ATmega2560, AVR_ATmega128RFA1, AVR_ATmega256RFR2 => EXTERNAL_NUM_INTERRUPTS == 8
#if (EXTERNAL_NUM_INTERRUPTS > 5)
                case INT5:
                  interruptHandler = handleINT5;
                  break;
                case INT6:
                  interruptHandler = handleINT6;
                  break;
                case INT7:
                  interruptHandler = handleINT7;
                  break;
#endif // EXTERNAL_NUM_INTERRUPTS > 5
                default:
                // still not attached
                extInterrupt[interruptNumber].mode = NOT_AN_INTERRUPT;
              }  // switch (interruptNumber)
              // check again to take in account 'No interrupt choosed' case
              if (NOT_AN_INTERRUPT != extInterrupt[interruptNumber].mode) {
                 // if pin still not INPUT_PULLUP - system will hang up
                 pinMode(arg[0], INPUT_PULLUP);
                 attachInterrupt(interruptNumber, interruptHandler, arg[2]);
                 // reinit counter
                 extInterrupt[interruptNumber].count = 0;
              }
            
           } // if (NOT_AN_INTERRUPT == extInterrupt[interruptNumber].mode)
           result = extInterrupt[interruptNumber].count;
         } // if ((EXTERNAL_NUM_INTERRUPTS > interruptNumber) && (RISING >= arg[2])) 
       } // if (isSafePin(arg[0]))
       break;
#endif // FEATURE_EXTERNAL_INTERRUPT_ENABLE

#ifdef FEATURE_ENCODER_ENABLE
    case CMD_ENCODER_COUNT:
      /*/
      /=/  incEnc.count[terminalAPin, terminalBPin, intNumber, initialValue]
      /*/
      // TODO: maybe need to rework code block
      // 
      if (isSafePin(arg[0]) && isSafePin(arg[1])) {
         int8_t interruptNumber=digitalPinToInterrupt(arg[0]);
         voidFuncPtr interruptHandler;
         // Interrupt number is correct?
         if (EXTERNAL_NUM_INTERRUPTS > interruptNumber) {
            // Interrupt mode is changed
            // just use NOT_AN_INTERRUPT = -1 macro from Arduino.h
           // Interrupt not attached?
           if (NOT_AN_INTERRUPT == extInterrupt[interruptNumber].mode) {
              extInterrupt[interruptNumber].mode = CHANGE;
              switch (interruptNumber) {
// Basic configuration => EXTERNAL_NUM_INTERRUPTS == 3
                case INT0:
                  interruptHandler = handleINT0ForEncoder;
                  break;
                case INT1:
                  interruptHandler = handleINT1ForEncoder;
                  break;
                default:
                // still not attached
                extInterrupt[interruptNumber].mode = NOT_AN_INTERRUPT;
              }  // switch (interruptNumber)
              // check again to take in account 'No interrupt choosed' case
              if (NOT_AN_INTERRUPT != extInterrupt[interruptNumber].mode) {
                 // if pin still not INPUT_PULLUP - system will hang up
                 pinMode(arg[0], INPUT_PULLUP);
                 pinMode(arg[1], INPUT_PULLUP);
                 attachInterrupt(interruptNumber, interruptHandler, CHANGE);
                 // reinit counter
                 extInterrupt[interruptNumber].encTerminalAPin = arg[0];
                 extInterrupt[interruptNumber].encTerminalBPin = arg[1];
                 extInterrupt[interruptNumber].count = arg[3];
              }
           } // if (NOT_AN_INTERRUPT == extInterrupt[interruptNumber].mode)
           result = extInterrupt[interruptNumber].count;
         } // if (EXTERNAL_NUM_INTERRUPTS > interruptNumber)
       } // ((isSafePin(arg[0]) && isSafePin(arg[1]))
       break;
#endif // FEATURE_ENCODER_ENABLE
     


#ifdef FEATURE_OW_ENABLE
    case CMD_OW_SCAN:
      /*/
      /=/  OW.scan[pin]
      /*/
      if (isSafePin(arg[0])) {
         result = oneWireScan(arg[0]);
      }
      break;
#endif // FEATURE_ONEWIRE_ENABLE


#ifdef FEATURE_I2C_ENABLE
    case CMD_I2C_SCAN:
      /*/
      /=/  I2C.scan[sdaPin, sclPin]
      /*/
      if (isSafePin(arg[0]) && isSafePin(arg[1])) {
         result = i2CScan();
      }
      break;

    case CMD_I2C_WRITE:
      /*/
      /=/ i2c.write(sdaPin, sclPin, i2cAddress, register, data)
      /*/
      if (isSafePin(arg[0]) && isSafePin(arg[1])) {
         // i2COption used as 'data'
         result = i2CWriteByte(i2CAddress, (('\0' != cBuffer[argOffset[3]]) ? i2CRegister : I2C_NO_REG_SPECIFIED), i2COption);
         result = (0 == result) ? RESULT_IS_OK : RESULT_IS_FAIL;
      }
      break;

    case CMD_I2C_READ:
      /*/
      /=/ i2c.read(sdaPin, sclPin, i2cAddress, register, length)
      /*/
      if (isSafePin(arg[0]) && isSafePin(arg[1])) {
         // i2COption used as 'length' - how much bytes must be read: 0..4 byte
         i2COption = constrain(i2COption, 1, 4);
         result = i2CReadBytes(i2CAddress, (('\0' != cBuffer[argOffset[3]]) ? i2CRegister : I2C_NO_REG_SPECIFIED), i2CValue, i2COption);
         // make int32 from i2C's bytes
         if (0 != result) {result = RESULT_IS_FAIL; break; }
         for (i=0; i < i2COption; i++) {
             result <<= 8;
             result |= i2CValue[i];
          }
      }
      break;

    case CMD_I2C_BITWRITE:
      /*/
      /=/  i2c.bitWrite(sdaPin, sclPin, i2cAddress, register, bitNumber, value)
      /*/
      if (isSafePin(arg[0]) && isSafePin(arg[1])) {
         // i2COption used as 'bit number'
         if (0 > i2COption || 7 < i2COption){
            result = RESULT_IS_FAIL;
            break;
         }
         // Use device's register if specified, read 1 byte
         result = i2CReadBytes(i2CAddress, (('\0' != cBuffer[argOffset[3]]) ? i2CRegister : I2C_NO_REG_SPECIFIED), i2CValue, 1);
         // "!!" convert value 0100 to 1.
         bitWrite (i2CValue[0], i2COption, (!!arg[5]));
         // Use device's register if specified, write 1 byte, returns Wire lib state
         result = i2CWriteByte(i2CAddress, (('\0' != cBuffer[argOffset[3]]) ? i2CRegister : I2C_NO_REG_SPECIFIED), i2CValue[0]);
         result = (0 == result) ? RESULT_IS_OK : RESULT_IS_FAIL;
      }
      break;
      
    case CMD_I2C_BITREAD:
      /*/
      /=/  i2c.bitRead(sdaPin, sclPin, i2cAddress, register, bit)
      /*/
      if (isSafePin(arg[0]) && isSafePin(arg[1])) {
         // i2COption used as 'bit number'
         if (0 > i2COption || 7 < i2COption){
            result = RESULT_IS_FAIL;
            break;
         }
         // Use device's register if specified, read 1 byte
         result = i2CReadBytes(i2CAddress, (('\0' != cBuffer[argOffset[3]]) ? i2CRegister : I2C_NO_REG_SPECIFIED), i2CValue, 1);
         result = bitRead(i2CValue[0], i2COption);
      }
      break;


#endif // FEATURE_I2C_ENABLE

#ifdef FEATURE_DS18X20_ENABLE
    case CMD_DS18X20_TEMPERATURE:
      /*/
      /=/  DS18x20.temperature[pin, resolution, id]
      /*/
      if (isSafePin(arg[0])) {
         result = DS18X20Read(arg[0], arg[1], &cBuffer[argOffset[2]], cBuffer);
      }
      break;
#endif // FEATURE_DS18X20_ENABLE

#ifdef FEATURE_DHT_ENABLE
    case CMD_DHT_HUMIDITY:
      /*/
      /=/  DHT.humidity[pin, model]
      /*/
      if (isSafePin(arg[0])) {
        result = DHTRead(arg[0], arg[1], SENS_READ_HUMD, cBuffer);
      }
      break;

    case CMD_DHT_TEMPERATURE:
      /*/
      /=/  DHT.temperature[pin, model]
      /*/
      if (isSafePin(arg[0])) {
        result = DHTRead(arg[0], arg[1], SENS_READ_TEMP, cBuffer);
      }
      break;
   
#endif // FEATURE_DHT_ENABLE

       
#ifdef FEATURE_BMP_ENABLE
    case CMD_BMP_PRESSURE:
      /*/
      /=/  BMP.Pressure[sdaPin, sclPin, i2cAddress, overSampling, filterCoef]
      /*/
      if (isSafePin(arg[0]) && isSafePin(arg[1])) {
          // (uint8_t) arg[2] is i2c address, 7 bytes size
         result = BMPRead(arg[0], arg[1], i2CAddress, arg[3], arg[4], SENS_READ_PRSS, cBuffer);
      }
      break;

    case CMD_BMP_TEMPERATURE:
      /*/
      /=/ BMP.Temperature[sdaPin, sclPin, i2cAddress, overSampling]
      /*/
      if (isSafePin(arg[0]) && isSafePin(arg[1])) {
         // (uint8_t) arg[2] is i2c address, 7 bytes size
         result = BMPRead(arg[0], arg[1], i2CAddress, arg[3], arg[4], SENS_READ_TEMP, cBuffer);
      }
      break;

      
#ifdef SUPPORT_BME280_INCLUDE
      case CMD_BME_HUMIDITY:
      /*/
      /=/  BME.Humidity[sdaPin, sclPin, i2cAddress, overSampling, filterCoef]
      /*/
      if (isSafePin(arg[0]) && isSafePin(arg[1])) {
          // (uint8_t) arg[2] is i2c address, 7 bytes size
         result = BMPRead(arg[0], arg[1], i2CAddress, arg[3], arg[4], SENS_READ_HUMD, cBuffer);
      }
      break;      
#endif // SUPPORT_BME280_INCLUDE 

#endif // FEATURE_BMP_ENABLE  


#ifdef FEATURE_BH1750_ENABLE
    case CMD_BH1750_LIGHT:
      /*/
      /=/  BH1750.light[sdaPin, sclPin, i2cAddress, mode]
      /*/
      if (isSafePin(arg[0]) && isSafePin(arg[1])) {
         // (uint8_t) arg[2] is i2c address, 7 bytes size
         result = BH1750Read(arg[0], arg[1], i2CAddress, arg[3], SENS_READ_LUX, cBuffer);
      }
      break;
#endif // FEATURE_BH1750_ENABLE

#ifdef FEATURE_MAX7219_ENABLE
    case CMD_MAX7219_WRITE:
      /*/
      /=/  MAX7219.write[dataPin, clockPin, loadPin, intensity, value]
      /*/
      if (isSafePin(arg[0]) && isSafePin(arg[1])  && isSafePin(arg[2])) {
         max7219DrawOn8x8(arg[0], arg[1], arg[2], arg[3], &cBuffer[argOffset[4]]);
         result = RESULT_IS_OK;
      }
      break;
#endif // FEATURE_MAX7219_ENABLE

#ifdef FEATURE_PCF8574_LCD_ENABLE
    case CMD_PCF8574_LCDPRINT:
      /*/
      /=/  PCF8574.LCDPrint[sdaPin, sclPin, i2cAddress, lcdBacklight, lcdType, data]
      /*/
      if (isSafePin(arg[0]) && isSafePin(arg[1])) {
         result = pcf8574LCDOutput(arg[0], arg[1], i2CAddress, arg[3], arg[4], &cBuffer[argOffset[5]]);
      }
      break;

#endif // FEATURE_PCF8574_LCD_ENABLE


#ifdef FEATURE_SHT2X_ENABLE
    case CMD_SHT2X_HUMIDITY:
      /*/
      /=/  SHT2X.Humidity[sdaPin, sclPin, i2cAddress]
      /*/
      if (isSafePin(arg[0]) && isSafePin(arg[1])) {
         // (uint8_t) arg[2] is i2c address, 7 bytes size
         result = SHT2XRead(arg[0], arg[1], i2CAddress, SENS_READ_HUMD, cBuffer);
      }
      break;

    case CMD_SHT2X_TEMPERATURE:
      /*/
      /=/  SHT2X.Temperature[sdaPin, sclPin, i2cAddress]
      /*/
      if (isSafePin(arg[0]) && isSafePin(arg[1])) {
         // (uint8_t) arg[2] is i2c address, 7 bytes size
         result = SHT2XRead(arg[0], arg[1], i2CAddress, SENS_READ_TEMP, cBuffer);
      }
      break;
#endif // FEATURE_SHT2X_ENABLE  



#ifdef FEATURE_ACS7XX_ENABLE
    case CMD_ACS7XX_ZC:
      /*/
      /=/  acs7xx.zc[sensorPin, refVoltage]
      /*/
      if (isSafePin(arg[0])) {
         /*
            for ATmega1280, ATmega2560, ATmega1284, ATmega1284P, ATmega644, ATmega644A, ATmega644P, ATmega644PA
               INTERNAL1V1 	2  - 1,1V
               INTERNAL2V56 	3  - 2,56V
            ATmega328 and so
               INTERNAL 	3  - 1,1V
            Both
               DEFAULT 	        1  - VCC
               EXTERNAL 	0  - AREF << mV on AREF, more than '3'
*/
         // if refVoltage skipped - use DEFAULT source
         if ('\0' != cBuffer[argOffset[1]]) {
           arg[1] = DEFAULT;
         }
         result = ACS7XXCurrent(arg[0], arg[1], SENS_READ_ZC, 0, 0, cBuffer);
      }
      break;

    case CMD_ACS7XX_AC:
      /*/
      /=/  acs7xx.ac[sensorPin, refVoltage, sensitivity, zeroPoint] 
      /*/
      if (isSafePin(arg[0])) {
         // if refVoltage skipped - use DEFAULT source
         if ('\0' != cBuffer[argOffset[1]]) {
           arg[1] = DEFAULT;
         }
         result = ACS7XXCurrent(arg[0], arg[1], SENS_READ_AC, arg[2], (int32_t) arg[3], cBuffer);
      }
      break;

    case CMD_ACS7XX_DC:
      /*/
      /=/  acs7xx.dc[sensorPin, refVoltage, sensitivity, zeroPoint] 
      /*/
      if (isSafePin(arg[0])) {
         // if refVoltage skipped - use DEFAULT source
         if ('\0' != cBuffer[argOffset[1]]) {
           arg[1] = DEFAULT;
         }
         result = ACS7XXCurrent(arg[0], arg[1], SENS_READ_DC, arg[2], (int32_t) arg[3], cBuffer);
      }
      break;

#endif // FEATURE_ACS7XX_ENABLE


#ifdef FEATURE_ULTRASONIC_ENABLE
    case CMD_ULTRASONIC_DISTANCE:
      /*/
      /=/  ultrasonic.distance[triggerPin, echoPin]
      /*/
      if (isSafePin(arg[0]) & isSafePin(arg[1])) {
         result = ultrasonicDistance(arg[0], arg[1]);
      }
      break;
#endif // FEATURE_ULTRASONIC_ENABLE


#ifdef FEATURE_IR_ENABLE
    case CMD_IR_SEND:
      /*/
      /=/  ir.send[pwmPin, irPacketType, nBits, data, repeat, address]
      /*/
      // ATmega328: Use D3 only at this time
      // Refer to other Arduino's pinouts to find OC2B pin
      if (isSafePin(arg[0]) && TIMER2B == digitalPinToTimer(arg[0])) {
         // irPWMPin - global wariable that replace IRremote's TIMER_PWM_PIN
         irPWMPin = arg[0];
         result = irSend(arg[1], arg[2], arg[3], arg[4], arg[5]);
      }
      break;

    case CMD_IR_SENDRAW:
      /*/
      /=/  ir.sendRaw[pwmPin, irFrequency, nBits, data]
      /=/  >> need to increase ARGS_PART_SIZE, because every data`s Integer number take _four_ HEX-chars => 70 RAW array items take 282 (2+70*4) byte of incoming buffer only
      /*/
      // ATmega328: Use D3 only at this time
      // Refer to other Arduino's pinouts to find OC2B pin
      if (isSafePin(arg[0]) && TIMER2B == digitalPinToTimer(arg[0])) {
         // irPWMPin - global wariable that replace IRremote's TIMER_PWM_PIN
         irPWMPin = arg[0];
         result = irSendRaw(arg[1], arg[2], &cBuffer[argOffset[3]]);
         result = (result) ? RESULT_IS_OK : RESULT_IS_FAIL;
      }
      break;
#endif // FEATURE_IR_ENABLE


    default:
      // In default case command  is considered unknown.
      strcpy(cBuffer, ZBX_NOTSUPPORTED_MSG);
      // Early increased command counter is decremented
      sysMetrics[IDX_METRIC_SYS_CMD_COUNT]--;
      result = RESULT_IN_BUFFER;
   }


   // The result is already printed?
   if (RESULT_IS_PRINTED != result) {
      // The result is placed to buffer?
      if (RESULT_IN_BUFFER != result) {
         //  '1' must be returned
         if (RESULT_IS_OK == result) {
            result = 1L;
         // or '0'
         } else if (RESULT_IS_FAIL == result) {
            result = 0L;
         }
         //  If result is number - convert its to C-string.
         ltoa (result, cBuffer, 10);
      }
      //  Push the buffer
      ethClient.println(cBuffer);
#ifdef FEATURE_DEBUG_TO_SERIAL
     SerialPrint_P(PSTR("Result: ")); Serial.println(cBuffer); Serial.println(); 
#endif
   }
   return cmdIdx;
}



