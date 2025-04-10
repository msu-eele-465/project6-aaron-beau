/*-----------------------------------------------------------------------------
* Project 6:Controlling a plant with the main controller and I2C
* Aaron Foster & Beau Coburn
* EELE 465
* 4/3/25
* 
* Main file for the controller msp. receives
* a character from the keypad to unlock. Later receives
* characters from the keypad and transmits commands to 
* the peripherals. Also reads LM92 temperature sensor and sends output to 
* LCD microcontroller as well as plant to adjust temperature accordingly
*///----------------------------------------------------------------------------




#include <msp430.h>
#include <stdint.h>
#include "intrinsics.h"
#include "src/keypad_scan.h"
#include "src/rgb_control.h"
#include "src/controller_control.h"
#include "src/adc_control.h"

#define MAX_WINDOW_SIZE 9           //max size that window can be

//---------------------- Variables ---------------------------------------------
int locked = 1;                                 // Locked Boolean
int relock = 0;                                 // Toggle to relock
volatile uint8_t *txData;                       // Pointer to data buffer
int SetOnce=1;                                  // Variable to trigger Tx once
int window_size = 8;
int window_size_unset;
//---------------------- i2c Variables -----------------------------------------
volatile char Packet[] = {0x00};                         // Tx Packet
int Data_Cnt = 0;                               // Used for multiple bytes sent
int i;                                          // Delay counter variable
//---------------------- ADC Variables -----------------------------------------
volatile unsigned int adc_value;                // Stores raw ADC reading (0-4095)
volatile uint16_t adc_samples[MAX_WINDOW_SIZE];  // Array to store ADC readings
volatile uint32_t adc_sum = 0;  // Sum of the last 'window_size' samples
volatile uint8_t sample_index = 0;  // Index for circular buffer
volatile float temperature_C = 0.0;  // Stores calculated temperature
volatile uint8_t samples_collected = 0;  // Tracks how many samples have been collecte

//--------------------- Plant temp variables -----------------------------------
volatile unsigned int plant_value;                // Stores raw ADC reading (0-4095)
volatile uint16_t plant_samples[MAX_WINDOW_SIZE];  // Array to store ADC readings
volatile uint32_t plant_sum = 0;  // Sum of the last 'window_size' samples
volatile uint8_t plant_index = 0;  // Index for circular buffer
volatile float plant_temperature_C = 0.0;  // Stores calculated temperature
volatile uint8_t plant_samples_collected = 0;  // Tracks how many samples have been collecte

//------------------------------------------------------------------------------
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;                   // Stop watchdog timer

UCB1CTLW0 &= ~UCSWRST;
controller_init();
ADC_init();
plant_temp_init();

__bis_SR_register(GIE);  // Enable global interrupts

    PM5CTL0 &= ~LOCKLPM5;  // Disable Low power mode
//------------------------ End Initialization ----------------------------------

/* 
* Main loop continually scans keypad and jumps to different while(locked) loops 
* Depending on the button pressed
*/
    while (1) {


/*
//--Locked                                 // While Locked variable is set  
        while (locked == 1) {              // Set led bar off and scan keypad
            if(SetOnce==1){
                Packet[0]=0x00;
                SetOnce=0;
                UCB1CTLW0 |= UCTXSTT;
            }

            rgb_control(1);
            locked = unlock_keypad();
        }
*/locked=0;
//--Unlocked                               // When locked variable is not set
        while (locked == 0) {              // continually scan keypad and tx
            rgb_control(3);                // Based on button press
            relock = led_pattern();

/* Set packet for tx, transmit, briefly change LED to green */

            switch(relock){
                case 0: UCB1I2CSA = 0x0069; Packet[0]=0x00; SetOnce=1; UCB1CTLW0 |= UCTXSTT; 
	                    for(i=0; i<100; i++){} UCB1I2CSA = 0x00E; UCB1CTLW0 |= UCTXSTT; 
                        rgb_control(2); __delay_cycles(500000); 
                        break;

                case 1: UCB1I2CSA = 0x0069; Packet[0]=0x01; SetOnce=1; UCB1CTLW0 |= UCTXSTT;
                        for(i=0; i<100; i++){} UCB1I2CSA = 0x00E; UCB1CTLW0 |= UCTXSTT; 
                        rgb_control(2); __delay_cycles(500000); 
                        if(window_size_unset == 1){
                            window_size = 1;
                            window_size_unset = 0;
                         }
                        break;

                case 2: UCB1I2CSA = 0x0069; Packet[0]=0x02; SetOnce=1; UCB1CTLW0 |= UCTXSTT;
                        for(i=0; i<100; i++){} UCB1I2CSA = 0x00E; UCB1CTLW0 |= UCTXSTT;
                        rgb_control(2); __delay_cycles(500000); 
                        if(window_size_unset == 1){
                            window_size = 2;
                            window_size_unset = 0;
                         }
                        break;
                        
                case 3: UCB1I2CSA = 0x0069; Packet[0]=0x03; SetOnce=1; UCB1CTLW0 |= UCTXSTT;
                        for(i=0; i<100; i++){} UCB1I2CSA = 0x00E; UCB1CTLW0 |= UCTXSTT; 
                        rgb_control(2); __delay_cycles(500000); 
                        if(window_size_unset == 1){
                            window_size = 3;
                            window_size_unset = 0;
                         }
                        break;

                case 4: UCB1I2CSA = 0x0069; Packet[0]=0x04; SetOnce=1; UCB1CTLW0 |= UCTXSTT;
                        for(i=0; i<100; i++){} UCB1I2CSA = 0x00E; UCB1CTLW0 |= UCTXSTT; 
                        rgb_control(2); __delay_cycles(500000); 
                        if(window_size_unset == 1){
                            window_size = 4;
                             window_size_unset = 0;
                         }
                        break;

                case 5: UCB1I2CSA = 0x00E; Packet[0] = 0x5; UCB1CTLW0 |= UCTXSTT;
                        rgb_control(2); __delay_cycles(500000); 
                        if(window_size_unset == 1){
                            window_size = 5;
                            window_size_unset = 0;
                         }
                        break;

                case 6: UCB1I2CSA = 0x00E; Packet[0] = 0x6; UCB1CTLW0 |= UCTXSTT;
                        rgb_control(2); __delay_cycles(500000); 
                        if(window_size_unset == 1){
                            window_size = 6;
                            window_size_unset = 0;
                         }
                        break;

                case 7: UCB1I2CSA = 0x00E; Packet[0] = 0x7; UCB1CTLW0 |= UCTXSTT;
                        rgb_control(2); __delay_cycles(500000); 
                        if(window_size_unset == 1){
                            window_size = 7;
                            window_size_unset = 0;
                         }
                        break;

                case 8: UCB1I2CSA = 0x00E; Packet[0] = 0x8; UCB1CTLW0 |= UCTXSTT;
                        rgb_control(2); __delay_cycles(500000); 
                        if(window_size_unset == 1){
                            window_size = 8;
                            window_size_unset = 0;
                         }
                        break;                

                case 9:  Packet[0]=0x9; SetOnce=1;
                          UCB1I2CSA = 0x00E; UCB1CTLW0 |= UCTXSTT; 
                         rgb_control(2); __delay_cycles(500000); 
                         if(window_size_unset == 1){
                            window_size = 9;
                            window_size_unset = 0;
                         }
                         break;

                case 0xA: Packet[0]=0xA; SetOnce=1; UCB1I2CSA = 0x00E; UCB1CTLW0 |= UCTXSTT;
                          rgb_control(2); __delay_cycles(500000);
                          window_size_unset = 1;
                          break;

                case 0xB: UCB1I2CSA = 0x0069; Packet[0]=0xB; SetOnce=1; UCB1CTLW0 |= UCTXSTT;
                        for(i=0; i<100; i++){} UCB1I2CSA = 0x00E; UCB1CTLW0 |= UCTXSTT; 
                        rgb_control(2); __delay_cycles(500000); break;

                case 0xC:  Packet[0]=0xC; SetOnce=1;
                          UCB1I2CSA = 0x00E; UCB1CTLW0 |= UCTXSTT; 
                         rgb_control(2); __delay_cycles(500000); break;

                case 0xD: UCB1I2CSA = 0x0069; Packet[0]=0xD; locked=1; SetOnce=1; UCB1CTLW0 |= UCTXSTT; 
                         for(i=0; i<100; i++){} UCB1I2CSA = 0x00E; UCB1CTLW0 |= UCTXSTT; 
                         rgb_control(2); __delay_cycles(500000); break;

                default: 
                    break;
            }
        }

    }

    return 0;
}
//------------------Interrupt Service Routines----------------------------------
/* ISR for I2C, iterates through Packet for each variable to be sent*/
#pragma vector = USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void) {
    if(Data_Cnt == (sizeof(Packet)-1)) {
        UCB1TXBUF = Packet[Data_Cnt];
        Data_Cnt = 0;
    }else{
        UCB1TXBUF = Packet[Data_Cnt];
        Data_Cnt++;
    }
}
// Timer_B ISR - Triggers ADC every 0.5s
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B_ISR(void) {
   if(locked == 0){
    ADCCTL0 |= ADCENC | ADCSC;  // Start ADC conversion
   }
    TB0CCTL0 &= ~CCIFG;  // Clear interrupt flag
}

/*ISR for reading ADC temperature, triggers every 0.5s from other ISR*/
#pragma vector = ADC_VECTOR
__interrupt void ADC_ISR(void)
{
    // Read plant and ambient temperature
    adc_value = ADCMEM0;  
    plant_value = read_plant_temp();

// ADC into moving average logic
   // Subtract the oldest sample from sum
    adc_sum -= adc_samples[sample_index];
    // Store new sample in array
    adc_samples[sample_index] = adc_value;
    // Add new sample to sum
    adc_sum += adc_value;
    // Move to next index, wrap around if necessary
    sample_index = (sample_index + 1) % window_size;
    // Ensure we have enough samples before averaging
    if (samples_collected < window_size) {
        samples_collected++;
    }
    // Calculate rolling average temperature (once enough samples are collected)
    if (samples_collected == window_size) {
        float conversion_factor = 20.05 / 2047;
        temperature_C = (adc_sum/window_size) * conversion_factor;
        Send_ADC(temperature_C);
    
    }
// Plant into moving average logic
// ADC into moving average logic
   // Subtract the oldest sample from sum
    plant_sum -= plant_samples[plant_index];
    // Store new sample in array
    plant_samples[plant_index] = plant_value;
    // Add new sample to sum
    plant_sum += plant_value;
    // Move to next index, wrap around if necessary
    plant_index = (plant_index + 1) % window_size;
    // Ensure we have enough samples before averaging
    if (plant_samples_collected < window_size) {
        plant_samples_collected++;
    }
    // Calculate rolling average temperature (once enough samples are collected)
    if (plant_samples_collected == window_size) {
        
        plant_temperature_C = (plant_sum/window_size);
        Send_ADC(plant_temperature_C);
    
    }


}
