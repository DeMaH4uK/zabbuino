#pragma once

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                              SYSTEM HEADERS SECTION
*/

#include <Arduino.h>
#include <IPAddress.h>
#include <SoftwareSerial.h>

#include <time.h>

#include <util/atomic.h>
#include <avr/interrupt.h>

#include <avr/pgmspace.h>
#include <wiring_private.h>


/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                               ZABBUINO HEADERS SECTION
*/

// cfg_basic.h & cfg_tune.h must be included before other zabbuino's headers because they contain configuration data
#include "sys_includes.h"

#include "SoftwareWire/SoftwareWire.h"
#include "NetworkAddress.h"
#include "network.h"
#include "sys_commands.h"
#include "sys_platforms.h"

// runtime libs 
#include "adc.h"
#include "eeprom.h"
#include "io_regs.h"
#include "rtc.h"
#include "system.h"
#include "service.h"
#include "plugin.h"

// I2C devices 
#include "i2c_bus.h"
#include "i2c_common.h"
#include "i2c_bh1750.h"
#include "i2c_ina2xx.h"
#include "i2c_lcd.h"
#include "i2c_sht.h"
#include "i2c_bmp.h"
#include "i2c_ds3231.h"
#include "i2c_pcf8563.h"
#include "i2c_at24cxx.h"
#include "i2c_max44009.h"
#include "i2c_veml6070.h"

// 1-Wire devices 
#include "ow_bus.h"
#include "ow_sensors.h"

// UART connected devices 
#include "uart_bus.h"
#include "uart_apcsmart.h"
#include "uart_megatec.h"
#include "uart_pzem.h"

// Other devices //
#include "dht.h"
#include "ir.h"
#include "interrupts.h"
#include "mh_zxx.h"
#include "ultrasonic.h"
#include "shiftout.h"
#include "microwire_bus.h"
#include "actuators.h"

/*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                   GLOBAL VARIABLES SECTION
*/

// some members of struct used in timer's interrupt
volatile sysmetrics_t sysMetrics;

netconfig_t netConfig;

NetworkClass Network;

#ifdef TWI_USE
SoftwareWire SoftTWI(constDefaultSDAPin, constDefaultSCLPin);
#endif

#ifdef INTERRUPT_USE
extern volatile extInterrupt_t extInterrupt[];
#endif

