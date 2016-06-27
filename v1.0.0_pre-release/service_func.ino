/* ****************************************************************************************************************************
*
*   Set default values of network configuration
*
**************************************************************************************************************************** */
void setDefaults(netconfig_t& _configStruct)
{
  char hostname[] = ZBX_AGENT_DEFAULT_HOSTNAME;  
  //  memcpy(_configStruct.hostname, hostname, ZBX_AGENT_HOSTNAME_MAXLEN-1);
  //_configStruct.hostname[ZBX_AGENT_HOSTNAME_MAXLEN]='\0';
  sethostname(_configStruct.hostname, hostname);
  uint8_t mac[] = NET_DEFAULT_MAC_ADDRESS;
  memcpy(_configStruct.macAddress, mac, sizeof(netConfig.macAddress));
  _configStruct.useDHCP = NET_DEFAULT_USE_DHCP;
  _configStruct.ipAddress = IPAddress(NET_DEFAULT_IP_ADDRESS);
  _configStruct.ipNetmask = IPAddress(NET_DEFAULT_NETMASK);
  _configStruct.ipGateway = IPAddress(NET_DEFAULT_GATEWAY);
//#ifdef PASSWORD_PROTECTION_FEATURE_ENABLE
  _configStruct.password = SYS_DEFAULT_PASSWORD;
  _configStruct.useProtection = SYS_DEFAULT_PROTECTION;
//#endif
}


void sethostname(char* _dest, const char* _src){
   strncpy(_dest, _src, ZBX_AGENT_HOSTNAME_MAXLEN-1);
   _dest[ZBX_AGENT_HOSTNAME_MAXLEN]='\0';
//   return _dest;

}

/* ****************************************************************************************************************************
*
*   Convert int32_t _number to char[]  with decimal point on _num_after_dot position 
*   _number / (10 * _num_after_dot position) => char[]
*
**************************************************************************************************************************** */
void ltoaf(int32_t _number, char* _dst, uint8_t _num_after_dot)
{
  uint8_t i, skipLeadingZeros = true;
  char currChar;
  uint32_t value = _number;
  const uint8_t maxStringLen = 10;
  const uint32_t dividers[maxStringLen]={1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
  
  // If Zero given - Zero returned without long processing 
  if (0 == value) { _dst[0] = '0';  _dst[1] = '\0'; return;} 
 
  // negative value testing and write '-' to output buffer
  if (0 > value) { value = 0 - value; *_dst = '-'; _dst++;} 
  
  // Use all dividers 
  for (i = 0; i < maxStringLen; i++) {
    // Its _num_after_dot-th position ?
    if ((maxStringLen - i) == _num_after_dot) {
        // If non-zero character has not yet processeed - push '0' before decimal point
        if (skipLeadingZeros) {*_dst = '0'; _dst++;}
        // push decimal point
        *_dst = '.'; _dst++; 
        // Need to process all next zeros
        skipLeadingZeros = false;
    }
    // Init character value
    currChar = '0';
    // If divider more than digit in current 'position' 
    // 100 <= 6xx
    while (dividers[i] <= value) {
      // Decrease that digit to next comparison (6xx, 5xx, 4xx, 3xx, 2xx, 1xx, 0xx)
      value -= dividers[i];
      // Increase character value
      currChar++;
    }
    // When the 'while' cycle is completed, the value of currChar will be increased N-th times
    // ('0', '1', '2', '3', '4', '5', '6') and currChar wil be represent digit that placed into
    // tested position.

    // All leadings Zeros must be skipped and do not written to output buffer
    if (currChar == '0' and skipLeadingZeros) { continue; }
    // Any non-zero sign processed - all following Zeros must be written to output buffer
    skipLeadingZeros = false; 
    // Push currChar to buffer
    *_dst = currChar;
    _dst++;
  }
  *_dst = '\0';
}

/* ****************************************************************************************************************************
*
*  Convert _len chars (exclude 0x prefix) of hex string to byte array
*
**************************************************************************************************************************** */
uint8_t hstoba(uint8_t* _array, const char* _data, uint8_t _len)
{
  // don't fill _array and return false if mailformed string detected
  if (!haveHexPrefix(_data)) { return false; }
  // skip prefix
  _data += 2;
  // for all bytes do...
  while (_len--)  {
     *_array = (htod(*_data) << 4);
     _data++;
     *_array += htod(*_data);
      _data++; _array++;      
  };
  return true;
}

/* ****************************************************************************************************************************
*
*   Compute a Dallas Semiconductor 8 bit CRC directly. This is much slower, but much smaller, than the lookup table.
*
*   This function placed here to aviod compilation error when OneWire library is not #included
*
**************************************************************************************************************************** */
uint8_t dallas_crc8(const uint8_t *addr, uint8_t len)
{
	uint8_t crc = 0;
	
	while (len--) {
		uint8_t inbyte = *addr++;
		for (uint8_t i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
}

/* ****************************************************************************************************************************
*
*  Print string stored in PROGMEM to Serial 
*
**************************************************************************************************************************** */
void SerialPrint_P (const char* _src) {
  char currChar;
  while ((currChar = pgm_read_byte(_src++)) != 0)
  Serial.print(currChar);
}

/* ****************************************************************************************************************************
*
*  Print string stored in PROGMEM to Serial + Line Feed
*
**************************************************************************************************************************** */
void SerialPrintln_P (const char* _src) {
  SerialPrint_P(_src);
  Serial.println();
}


/* ****************************************************************************************************************************
*
*   Convert array of char with given lenght to uint32_t variable
*
**************************************************************************************************************************** */
uint32_t pl_atol(char const *_src, uint8_t _len) {
  long result = 0;
  uint8_t i;
  int8_t sign = 1;

  // Check lengh of buffer and work if its no more max digits of int_32 type (11 chars)
  if ( 11 < _len ) {return 0; }
  
  // Sign of number is exist? 
  if ('-' == (*_src)) {sign = -1; _src++; _len--; }

  // Walk thru buffer and calculate int32_t number
  for (i = 0 ; i < _len ; i++) {
    if ((*_src < '0') or (*_src > '9')) { return 0; }
    result = (result << 3) + ( result<<1 ) + *_src -'0';
   _src++;
  }

  // If number with sign
  return sign * result;
}


/* ****************************************************************************************************************************
*
*  Print array to Serial as MAC or IP or other string with sign-separated parts
*
**************************************************************************************************************************** */
void printArray(uint8_t *_src, uint8_t _len, uint8_t _type)
{
  while (_len--) {
    if (DBG_PRINT_AS_MAC == _type) {
       Serial.print(*_src, HEX);
       if (1 <= _len) { Serial.print(':'); }
    } else if (DBG_PRINT_AS_IP == _type) {
       Serial.print(*_src, DEC);
       if (1 <= _len) Serial.print('.');
    }
    _src++;
  }
  Serial.println();
}

void blinkMore(uint8_t _times, uint16_t _onTime, uint16_t _offTime) 
{
  for (byte i=0; i < _times ; i++) {
    digitalWrite(PIN_STATE_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(_onTime);              // wait for a second
    digitalWrite(PIN_STATE_LED, LOW);   // turn the LED on (HIGH is the voltage level)
    delay(_offTime);              // wait for a second
  }
}
