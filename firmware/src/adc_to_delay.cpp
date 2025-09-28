#include "adc_to_delay.h"

#define NLOBITS 7       // 10-bit ADC val, 3 upper and 7 lower
#define MINTCKS 36      // lowest number of ticks (*0.213ms)

// Helper: Map ADC value to delay (user can adjust this formula)
uint16_t adc_to_delay(uint16_t adc_val) 
{
    uint8_t ubits = (adc_val >> NLOBITS) & 0x07;
    uint8_t lbits = adc_val & 0x007F;

    uint32_t kd = (1 << ubits);
    kd *= MINTCKS;
    kd += (kd * lbits) >> NLOBITS;
    
    return uint16_t(kd);
}
