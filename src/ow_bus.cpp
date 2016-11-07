#include "ow_bus.h"

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
                                                                     COMMON 1-WIRE SECTION
*/

/*
int32_t DS18X20Discovery(uint8_t _pin)
{
  uint8_t i, dsAddr[8],;

  OneWire ow(_pin);
  ethClient.print("{\"data\":[");

  ow.reset_search();
  ow.reset();
  i=0;
  while (!ow.search(dsAddr))
  {
    i++;
    ethClient.print("{\"{#INDEX}\":\""); ethClient.print(i); ethClient.print("\",")
    ethClient.print("\"{#ID}\":\""); ethClient.print(dsAddr[0]); ethClient.print("\",")
    ethClient.print("\"{#ADDR}\":\""); ethClient.print(addr-to-str(dsAddr[0])); ethClient.print("\"},")
  }
  ethClient.println("]}");
}
*/


/* ****************************************************************************************************************************
*
*   Scan 1-Wire bus and print to ethernet client all detected ID's (Addresses)
*
**************************************************************************************************************************** */
int8_t scanOneWire(const uint8_t _pin, EthernetClient *_ethClient) {
  uint8_t dsAddr[8], numDevices = 0, i;
  OneWire *owDevice;
  owDevice = new OneWire(_pin);
  owDevice->reset_search();
  delay(250);
  owDevice->reset();
  while (owDevice->search(dsAddr)) {
    numDevices++;
    _ethClient->print("0x");
    for (i = 0; i < arraySize(dsAddr); i++ ) {
      if (dsAddr[i] < 0x0F){ _ethClient->print("0"); }
      _ethClient->print(dsAddr[i], HEX);
    }
    _ethClient->print('\n');
  }
  delete owDevice;
  return ((0 < numDevices)  ? RESULT_IS_PRINTED : RESULT_IS_FAIL);
}


