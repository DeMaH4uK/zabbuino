uint8_t saveConfig(netconfig_t* _configStruct)
{
  uint8_t index, calculatedCRC, *p_configStruct = (uint8_t*) _configStruct;
  // Save every byte of _configStruct to EEPROM
  for (index=0; index < sizeof(netconfig_t); index++) {
    EEPROM[index] = p_configStruct[index];
  }

  // Calculate CRC of _configStruct. At this point index == sizeof(netconfig_t), no need to calculate its again
  calculatedCRC = dallas_crc8 (p_configStruct, index);
  // Store its to EEPROM at next position
  EEPROM[index+1]=calculatedCRC;
}

uint8_t loadConfig(netconfig_t* _configStruct)
{
  // EEPROM.begin() - для ESP8266
  // http://esp8266.ru/arduino-ide-esp8266/#eeprom
  uint8_t index, storedCRC, calculatedCRC, *p_configStruct = (uint8_t*) _configStruct;

  // Load _configStruct from EEPROM
  for (index = 0; index < sizeof(netconfig_t); index++) {
    // no sense to call AVR functions directly - sketch still uses the same size of program storage space and the same size of dynamic memory 
    // p_configStruct[index] = eeprom_read_byte((uint8_t *) index);
    // EEPROM[index] eq EEPROM.read(index);
    p_configStruct[index] = EEPROM[index];
  }

  // Calculate CRC of _configStruct which load from EEPROM. . At this point index == sizeof(netconfig_t), no need to calculate its again
  calculatedCRC = dallas_crc8 (p_configStruct, index);
  // Read _configStruct CRC which early stored to EEPROM
  storedCRC = EEPROM[index+1];
  // Check integrity of loaded structure...
  if (storedCRC == calculatedCRC) {
     // True, if loading finished successfully
     return true;
  } else {
     // Otherwise - False
     return false;
  }
}


