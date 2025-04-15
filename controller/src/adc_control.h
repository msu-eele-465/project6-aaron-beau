#ifndef adc_control_H
#define adc_control_H

#include "src/adc_control.h"
#include <msp430.h>
#include <stdint.h>

// Function prototype

void ADC_init();
void send_plant_temp(int value);
void Send_ADC(int value);
#endif
