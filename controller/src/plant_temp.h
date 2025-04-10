#ifndef adc_control_H
#define adc_control_H

#include "src/plant_temp.h"
#include <msp430.h>
#include <stdint.h>

// Function prototype

void temp_init();
int read_plant_temp();
#endif
