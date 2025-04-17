/*----------------------------------------------------------------------------------
Beau Coburn & Aaron Foster
EELE 465
2/25/25
Project 3: Keypad LED Pattersn
Description:  




-------------------------------------------------------------------------------------*/
#include <msp430.h> 
#include "adc_control.h"
#include "intrinsics.h"
#include <stdint.h>



void ADC_init(){

    P5SEL1 |= BIT0;  // Set P5.0 as ADC input
    P5SEL0 |= BIT0;

    ADCCTL0 &= ~ADCSHT;         //Clear ADCSHT
    ADCCTL0 |=  ADCSHT_2;       //Conversion Cycles = 16
    ADCCTL0 |= ADCON;           //turn ADC ON
    ADCCTL1 |= ADCSHP;          //sample signal source = sampling timer
    ADCCTL2 &= ~ADCRES;         //Clear ADCRES
    ADCCTL2 |= ADCRES_2;        // Resoultion  = 12 bit
    ADCMCTL0 |= ADCINCH_8;      //ADC INPUT Channel = A8
   
    ADCIE |= ADCIE0;            //enable ADC IRQ
   
    
}



