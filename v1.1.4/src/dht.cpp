/*
Based on: https://github.com/RobTillaart/Arduino/tree/master/libraries/DHTstable
version 0.1.13 is used

*/

#include "dht.h"

/*****************************************************************************************************************************
*
*  Read specified metric's value of the AM/DHT sensor, put it to output buffer on success. 
*
*   Returns: 
*     - RESULT_IN_BUFFER on success
*     - DEVICE_ERROR_CONNECT on connection error
*     - DEVICE_ERROR_ACK_L
*     - DEVICE_ERROR_ACK_H
*     - DEVICE_ERROR_TIMEOUT if sensor stops answer to the request
*
*****************************************************************************************************************************/
int8_t getDHTMetric(const uint8_t _pin, const uint8_t _sensorModel, const uint8_t _metric, char *_dst)
{
  int8_t rc = DEVICE_ERROR_TIMEOUT;
  // INIT BUFFERVAR TO RECEIVE DATA
  uint8_t mask = 128,
          idx = 0, 
          sum = 0,
          wakeupDelay,
          bits[5];  // buffer to receive data
  uint16_t loopCount = 0;
  uint32_t humidity, temperature, result,
           waitTime = 0;

  static uint32_t lastReadTime = 0;
  
  switch (_sensorModel) {
    case DHT11_ID:
      wakeupDelay = DHTLIB_DHT11_WAKEUP;
      break;
    case DHT21_ID:
    case DHT22_ID:
    case DHT33_ID:
    case DHT44_ID:
    default:
      wakeupDelay = DHTLIB_DHT_WAKEUP;
  }

  uint8_t bit = digitalPinToBitMask(_pin);
  uint8_t port = digitalPinToPort(_pin);
  volatile uint8_t *PIR = portInputRegister(port);

  // EMPTY BUFFER
  for (uint8_t i = 0; i < 5; i++) bits[i] = 0;
 
  // DHT sensor have limit for taking samples frequency - 1kHz (1 sample/sec)
  waitTime = millis() - lastReadTime;
  waitTime = (waitTime < 1100) ? (1100 - waitTime) : 0;

  pinMode(_pin, OUTPUT);

  // Sensor won't to connect if no HIGH level exist on pin for a few time (500ms at least)

  //  if (digitalRead(_pin) == LOW ) {
  if ((*PIR & bit) == LOW ) {
     digitalWrite(_pin, HIGH);
     if (waitTime < 500) {waitTime = 500;}
  }

  delay(waitTime); 

  stopTimerOne(); 
  // REQUEST SAMPLE
  digitalWrite(_pin, LOW);
  delay(wakeupDelay);
  digitalWrite(_pin, HIGH);
  delayMicroseconds(40);
  pinMode(_pin, INPUT);

  // data exchange with DHT sensor must not be interrupted

  // GET ACKNOWLEDGE or TIMEOUT
  loopCount = DHTLIB_TIMEOUT;
//  while (digitalRead(_pin) == LOW) {
  while ((*PIR & bit) == LOW) {
    if (--loopCount == 0) { rc = DEVICE_ERROR_ACK_L; goto finish; }
  }

  loopCount = DHTLIB_TIMEOUT;
//  while (digitalRead(_pin) == HIGH) {
  while ((*PIR & bit) != LOW) {
    if (--loopCount == 0) { rc = DEVICE_ERROR_ACK_H; goto finish; }
  }

    // READ THE OUTPUT - 40 BITS => 5 BYTES
    for (uint8_t i = 40; i != 0; i--)
    {
        loopCount = DHTLIB_TIMEOUT;
        //  while (digitalRead(_pin) == LOW) {
        while((*PIR & bit) == LOW) {
          if (--loopCount == 0) { rc = DEVICE_ERROR_TIMEOUT; goto finish; }  
        }

        uint32_t t = micros();

        loopCount = DHTLIB_TIMEOUT;
        //  while (digitalRead(_pin) == HIGH) {
        while((*PIR & bit) != LOW) {
          if (--loopCount == 0) { rc = DEVICE_ERROR_TIMEOUT; goto finish; }  
        }

        if ((micros() - t) > 40) { 
            bits[idx] |= mask;
        }
        mask >>= 1;
        if (mask == 0)   // next byte?
        {
            mask = 128;
            idx++;
        }
    }
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, HIGH);

  // Store time to use it to use on the next call of sub
  lastReadTime = millis();
     
  switch (_sensorModel) {
    case DHT11_ID:
      // original code part:
      //        these bits are always zero, masking them reduces errors.
      //        bits[0] &= 0x7F;
      //        bits[2] &= 0x7F;
      sum = bits[0] + bits[2];
      humidity    = bits[0] * 10;  // bits[1] == 0;
      temperature = bits[2] * 10;  // bits[3] == 0;
      break;
    case DHT21_ID:
    case DHT22_ID:
    case DHT33_ID:
    case DHT44_ID:
    default:
      // original code part:
      //        these bits are always zero, masking them reduces errors.
      //        bits[0] &= 0x03;
      //        bits[2] &= 0x83;
      //        humidity = (bits[0]*256 + bits[1]) * 0.1;  <== * 0.1 processed by ltoaf()
      sum = bits[0] + bits[1] + bits[2] + bits[3];
      humidity = (bits[0]*256 + bits[1]);
      temperature = ((bits[2] & 0x7F)*256 + bits[3]);// * 0.1;
      if (bits[2] & 0x80)  // negative temperature
         {
           temperature = -temperature;
         }
  }
  // TEST CHECKSUM
  if (bits[4] != sum) { rc = DEVICE_ERROR_CHECKSUM; goto finish; }

  result = (SENS_READ_HUMD == _metric) ? humidity : temperature;
  ltoaf(result, _dst, 1);
  rc = RESULT_IN_BUFFER;

  finish:
  startTimerOne(); 
  gatherSystemMetrics(); 
  return rc;
}


