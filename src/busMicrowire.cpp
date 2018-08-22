/* 
The original code was written for the Wiring board by:
 * Nicholas Zambetti and Dave Mellis /Interaction Design Institute Ivrea /Dec 2004
 * http://www.potemkin.org/uploads/Wiring/MAX7219.txt
 
First modification by:
 * Marcus Hannerstig/  K3, malmö högskola /2006
 * http://www.xlab.se | http://arduino.berlios.de
 
Second modification is by:
 * tomek ness /FH-Potsdam / Feb 2007
 * http://design.fh-potsdam.de/

*/

// Config & common included files
#include "sys_includes.h"

#include "service.h"
#include "system.h"

#include "busMicrowire.h"


/*****************************************************************************************************************************
*
*  Send one byte to MAX7219 controller
*
*  Returns: 
*    - none
*
*****************************************************************************************************************************/
static void writeByteToMAX7219(const uint8_t _dataPin, const uint8_t _clockPin, const uint8_t _data) 
{
  int8_t i = 7;
  while(i >= 0) {
    digitalWrite(_clockPin, LOW);                             // tick
//    digitalWrite(_dataPin, !!(_data & (0x01 << i)));        // send i-th bit value 
    digitalWrite(_dataPin, ((_data & (0x01 << i)) ? 1 : 0));  // send i-th bit value 
    digitalWrite(_clockPin, HIGH);                            // tock
    --i;                                                      // move to lesser bit
  }
}

/*****************************************************************************************************************************
*
*  Push one byte of data to MAX7219 controller
*
*  Returns: 
*    - none
*
*****************************************************************************************************************************/
static void pushDataToMAX7219(const uint8_t _dataPin, const uint8_t _clockPin, const uint8_t _loadPin, const uint8_t _register, const uint8_t _data) {    
  digitalWrite(_loadPin, LOW);
  // specify register or column
  writeByteToMAX7219(_dataPin, _clockPin, _register);   
  // put data  
  writeByteToMAX7219(_dataPin, _clockPin, _data);
  // show it
  digitalWrite(_loadPin, LOW);
  digitalWrite(_loadPin,HIGH);
}

/*****************************************************************************************************************************
*
*  Print incoming data on MAX7219 based digital led indicator or draw on led matrix.
*
*  Returns: 
*    - none
*
*****************************************************************************************************************************/
void writeToMAX7219(const uint8_t _dataPin, const uint8_t _clockPin, const uint8_t _loadPin, const uint8_t _intensity, char* _src) {    
  uint8_t col, currByte = 0,  isHexString = false;
  // Init the module 
  // Mark all columns as active
  pushDataToMAX7219(_dataPin, _clockPin, _loadPin, MAX7219_REGISTER_SCANLIMIT, 0x07);      
  // No decode digits - led matrix mode, define active led segsments manually
  pushDataToMAX7219(_dataPin, _clockPin, _loadPin, MAX7219_REGISTER_DECODE_MODE, 0x00);
  // Switch on IC
  pushDataToMAX7219(_dataPin, _clockPin, _loadPin, MAX7219_REGISTER_SHUTDOWN, 0x01);
  // Switch off display test
  pushDataToMAX7219(_dataPin, _clockPin, _loadPin, MAX7219_REGISTER_DISPLAYTEST, 0x00); // no display test
  // Set intensity
  pushDataToMAX7219(_dataPin, _clockPin, _loadPin, MAX7219_REGISTER_INTENSITY, _intensity & 0x0F);    // the first 0x0f is the value you can set

  // Draw line by line from first column...
  col = 1;
  // MAX7219.write[4,5,6,1,"1.2.3.4.5.6.7.8"]
  // MAX7219.write[4,5,6,1,"    1.25"]
  // MAX7219.write[15,16,17,1,"Hc  -1.25"]
  // HEX strings must be processeed specially
  if (haveHexPrefix(_src)) {
     // Skip "0x"
     _src += 2;
     isHexString = true;
  } else {

    uint8_t dataBufferSwapPosition = 0;
    uint8_t lenOfBuffer = 0;
    while (_src[lenOfBuffer]) { lenOfBuffer++; }
    uint8_t dataBufferPosition = lenOfBuffer-1;
    lenOfBuffer = lenOfBuffer >> 1;
    // 
    while (lenOfBuffer){
       // swap buffer items 
       char tmpVal = _src[dataBufferSwapPosition];
       _src[dataBufferSwapPosition] = _src[dataBufferPosition];
       _src[dataBufferPosition] = tmpVal;
       // shrink swapping area
       dataBufferPosition--;
       dataBufferSwapPosition++;
       lenOfBuffer--;
     } 
  }
  
  while (*_src) {
    // HEX processing
    if (isHexString) {
       // Make first four bits of byte to push from HEX.
       currByte = htod(*_src); _src++;
       // Move first nibble to high
       currByte <<= 4;
       // Check for second nibble existience
       if (*_src) {
          // Add its to byte if HEX not '\0'
          currByte |= htod(*_src);
       }
#ifndef NO_ASCII_SUPPORT
    } else {
      //
      //  ASCII processing
      //
      // currByte '1111110' =>  LED SEG 'ABCDEFG' , if DP must be fired up - currByte |= 0x80
      //    AAAA
      //   F    B
      //   F    B    
      //    GGGG
      //   E    C
      //   E    C
      //    DDDD   DP
      //
      char currChar=(char) *_src;

         if ('0' == currChar || 'O' == currChar) {
            currByte = B1111110;
         } else if ('1' == currChar) {
            currByte = B0110000;

         } else if ('2' == currChar) {
            currByte = B1101101;

         } else if ('3' == currChar) {
            currByte = B1111001;

         } else if ('4' == currChar) {
            currByte = B0110011;

         } else if ('5' == currChar) {
            currByte = B1011011;

         } else if ('6' == currChar) {
            currByte = B1011111;

         } else if ('7' == currChar) {
            currByte = B1110000;

         } else if ('8' == currChar) {
            currByte = B1111111;

         } else if ('9' == currChar) {
            currByte = B1111011;

         } else if ('-' == currChar) {
            currByte = B0000001;

         } else if ('_' == currChar) {
            currByte = B0001000;

         } else if ('A' == currChar) {
            currByte = B1110111;

         } else if ('b' == currChar) {
            currByte = B0011111;

         } else if ('C' == currChar) {
            currByte = B1001110;

         } else if ('c' == currChar) {
            currByte = B0001101;

         } else if ('d' == currChar) {
            currByte = B0111101;

         } else if ('H' == currChar) {
            currByte = B0110111;

         } else if ('h' == currChar) {
            currByte = B0010111;

         } else if ('E' == currChar) {
            currByte = B1001111;

         } else if ('L' == currChar) {
            currByte = B0001110;

         } else if ('l' == currChar) {
            currByte = B0000110;

         } else if ('P' == currChar) {
            currByte = B1100111;

         } else if ('n' == currChar) {
            currByte = B0010101;

         } else if ('o' == currChar) {
            currByte = B0011101;

         } else if ('r' == currChar) {
            currByte = B0000101;

         } else if ('t' == currChar) {
            currByte = B0001111;

         } else if ('u' == currChar) {
            currByte = B0011100;

         } else if ('U' == currChar) {
            currByte = B0111110;

         } else if ('.' == currChar) {
            // dot just skipped and processed on next step
            goto next;
         } else  {
         // if (0x20 == currChar || unknown == currChar)
            currByte = B0000000;
         } 
/*

      switch (currChar) {
         case '0':
         case 'O':
            currByte = B1111110;
            break;
         case '1':
            currByte = B0110000;
            break;
         case '2':
            currByte = B1101101;
            break;
         case '3':
            currByte = B1111001;
            break;
         case '4':
            currByte = B0110011;
            break;
         case '5':
            currByte = B1011011;
            break;
         case '6':
            currByte = B1011111;
            break;
         case '7':
            currByte = B1110000;
            break;
         case '8':
            currByte = B1111111;
            break;
         case '9':
            currByte = B1111011;
            break;
         case '-':
            currByte = B0000001;
            break;
         case '_':
            currByte = B0001000;
            break;
         case 'A':
            currByte = B1110111;
            break;
         case 'b':
            currByte = B0011111;
            break;
         case 'C':
            currByte = B1001110;
            break;
         case 'c':
            currByte = B0001101;
            break;
         case 'd':
            currByte = B0111101;
            break;
         case 'H':
            currByte = B0110111;
            break;
         case 'h':
            currByte = B0010111;
            break;
         case 'E':
            currByte = B1001111;
            break;
         case 'L':
            currByte = B0001110;
            break;
         case 'l':
            currByte = B0000110;
            break;
         case 'P':
            currByte = B1100111;
            break;
         case 'n':
            currByte = B0010101;
            break;
         case 'o':
            currByte = B0011101;
            break;
         case 'r':
            currByte = B0000101;
            break;
         case 't':
            currByte = B0001111;
            break;
         case 'u':
            currByte = B0011100;
            break;
         case 'U':
            currByte = B0111110;
            break;
         case '.':
            // dot just skipped and processed on next step
            goto next;
            break;
         case 0x20:
         default:
            currByte = B0000000;
            break;
     } // switch 
*/
      // 'dot' sign is next? 
      if ('.' == ((char) *(_src-1))) {currByte |= 0x80; }
#endif
    }
    // Pushing byte to column
    pushDataToMAX7219(_dataPin, _clockPin, _loadPin, col, currByte);
    // only 8 columns must be processeed, comment its if need more
    ++col;
    if (0x08 < col) { break; }
    next:
    ++_src;

  }
  gatherSystemMetrics(); // Measure memory consumption
  
}


