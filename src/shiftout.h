#ifndef _ZABBUINO_SHIFTOUT_H_
#define _ZABBUINO_SHIFTOUT_H_

#include <util/atomic.h>
#include "../basic.h"
#include "tune.h"
#include "service.h"
#include "system.h"


/*****************************************************************************************************************************
*
*  Do bit-bang on the specified pin. Sub can get HEX-string as the data to shift out
*
*   Returns: 
*     - none
*
*****************************************************************************************************************************/
void shiftOutAdvanced(const uint8_t _dataPin, const uint8_t _clockPin, const uint8_t _bitOrder, char *_src);

/*****************************************************************************************************************************
*
*  Push bitstream to WS2812 chip(s). Sub can get HEX-string as the data to shift out
*
*   Returns: 
*     - none
*
*****************************************************************************************************************************/
void WS2812Out(const uint8_t _dataPin, char *_src);

/*****************************************************************************************************************************
*
*  Prepare buffer for fast bit-banging. Convert byte to two nibble if need to push DEC-value. 
*  Convert HEX-string to array of nibbles. Reverse array of nibbles (and reverse bit order in nibbles) if bitOrder is LSBFIRST.
*
*  Returns: 
*    - number of bytes in the prepared data buffer
*
*****************************************************************************************************************************/
static uint16_t prepareBufferForAdvShiftout(const uint8_t _bitOrder, char *_src);


#endif // #ifndef _ZABBUINO_SHIFTOUT_H_