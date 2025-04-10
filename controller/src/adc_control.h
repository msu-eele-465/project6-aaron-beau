#ifndef adc_control_H
#define adc_control_H

#include "src/adc_control.h"
#include <msp430.h>
#include <stdint.h>

// Function prototype

void Convert_and_Send_Float(float value);
void ADC_init();
#endif
