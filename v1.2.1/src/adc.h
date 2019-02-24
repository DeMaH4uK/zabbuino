#ifndef _ZABBINO_ADC_H_
#define _ZABBINO_ADC_H_

#include "../basic.h"
#include "tune.h"
#include "service.h"
#include "system.h"


// for median sampling algo: every sample is int16_t number, total memory consumption is (ADC_SAMPLES * 2) bytes
#define ADC_SAMPLES                         1000  

/*****************************************************************************************************************************
*
*   Measure the voltage on specified analog channel.
*
*   Returns: 
*     - Voltage in mV
*
*****************************************************************************************************************************/
uint16_t getADCVoltage(const uint8_t);


/*****************************************************************************************************************************
*
*  Read specified metric's value of the ACS712 sensor, put it to output buffer on success. 
*
*  Returns: 
*    - always RESULT_IN_BUFFER 
*
*  Note: code is not tested in production
*
*****************************************************************************************************************************/
int8_t getACS7XXMetric(const uint8_t, uint32_t, const uint8_t, const uint8_t, const int32_t, char*);

#endif // #ifndef _ZABBINO_ADC_H_