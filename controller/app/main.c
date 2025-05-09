/*-----------------------------------------------------------------------------
* Project 6: Controlling a plant with the main controller and I2C
* Aaron Foster & Beau Coburn
* EELE 465
* 4/3/25
* 
* Main file for the controller msp. receives
* a character from the keypad to unlock. Later receives
* characters from the keypad and transmits commands to 
* the peripherals. Also reads LM92 digital temperature sensor and LM19 analog 
* temperature sensor and sends output to LCD microcontroller as well as plant 
* to adjust temperature accordingly.
*///----------------------------------------------------------------------------




#include <msp430.h>
#include <stdint.h>
#include "intrinsics.h"
#include "src/keypad_scan.h"
#include "src/rgb_control.h"
#include "src/controller_control.h"
#include "src/adc_control.h"
#include "src/lcd_control.h"

#define MAX_WINDOW_SIZE 9           //max size that window can be

//---------------------- Variables ---------------------------------------------
volatile uint8_t *txData;                       // Pointer to data buffer
int SetOnce=1;                                  // Variable to trigger Tx once
int window_size = 3;
//---------------------- i2c Variables -----------------------------------------
volatile char Packet[] = {0x00};                         // Tx Packet
int Data_Cnt = 0;                               // Used for multiple bytes sent
int i;                                          // Delay counter variable
//---------------------- ADC Variables -----------------------------------------
volatile unsigned int adc_value;              // Stores raw ADC reading (0-4095)
volatile uint16_t adc_samples[MAX_WINDOW_SIZE]; // Array to store ADC readings
volatile uint32_t adc_sum = 0;          // Sum of the last 'window_size' samples
volatile uint8_t sample_index = 0;              // Index for circular buffer
volatile float temperature_C = 0.0;             // Stores calculated temperature
volatile uint8_t samples_collected = 0;         // Tracks how many samples
                                                // have been collected

//--------------------- Plant temp variables -----------------------------------
volatile unsigned int plant_value;            // Stores raw ADC reading (0-4095)
volatile uint16_t plant_samples[MAX_WINDOW_SIZE];// Array to store ADC readings
volatile uint32_t plant_sum = 0;         // Sum of the last 'window_size' samples
volatile uint8_t plant_index = 0;               // Index for circular buffer
volatile float plant_temperature_C = 0.0;       // Stores calculated temperature
volatile uint8_t plant_samples_collected = 0;   // Tracks how many samples 
                                                // have been collected
int plant_mode;
int tens;
int ones;
int decimal;
//------------------------------------------------------------------------------
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;                   // Stop watchdog timer

UCB1CTLW0 &= ~UCSWRST;
controller_init();
ADC_init();
plant_temp_init();
LCD_init();
LCD_setup();
LCD_command(0xC0);
LCD_print("3", 1);
LCD_command(0x88); // move to row 1, position 9
LCD_print("A:", 2);
LCD_command(0xC8); // move to row 2, position 9
LCD_print("P:", 2);

__bis_SR_register(GIE);  // Enable global interrupts

    PM5CTL0 &= ~LOCKLPM5;  // Disable Low power mode
//------------------------ End Initialization ----------------------------------

/* 
* Main loop continually scans keypad and jumps to different I2C send messages 
* Depending on the button pressed
*/
    while (1) {
            rgb_control(3);                // Based on button press
            plant_mode = led_pattern();

/* Set packet for tx, transmit, briefly change LED to green */

            switch(plant_mode){

                case 0xA: UCB1I2CSA = 0x0069; Packet[0]=0xA; SetOnce=1;
                        Data_Cnt = 0;          //ensure count is zero
                        UCB1TBCNT = 1;         // set packet length to 1
                        UCB1CTLW0 |= UCTR;     // Transmitter mode
                        UCB1IE |= UCTXIE0;     // Enable TX interrupt
                        UCB1CTLW0 |= UCTXSTT;  // Start transmission
                        for(i=0; i<100; i++){} 
                        rgb_control(2); __delay_cycles(500000); 
                        LCD_clear_first_line(5);
                        LCD_print("Heat", 4);
                        P4OUT &= ~BIT3;         //set to heat
                        P4OUT &= ~BIT2;
                        P4OUT |= BIT2;
                        break;

                case 0xB: UCB1I2CSA = 0x0069; Packet[0]=0xB; SetOnce=1;
                        Data_Cnt = 0;          //ensure count is zero
                        UCB1TBCNT = 1;         // set packet length to 1
                        UCB1CTLW0 |= UCTR;     // Transmitter mode
                        UCB1IE |= UCTXIE0;     // Enable TX interrupt
                        UCB1CTLW0 |= UCTXSTT;  // Start transmission
                        for(i=0; i<100; i++){} 
                        rgb_control(2); __delay_cycles(500000); 
                        LCD_clear_first_line(5);
                        LCD_print("Cool", 4);
                         P4OUT &= ~BIT2;       //set to cool  
                         P4OUT &= ~BIT3;   
                         P4OUT |= BIT3;
                        break;

                case 0xC:  
                         rgb_control(2); __delay_cycles(500000);
                         LCD_clear_first_line(5); 
                         LCD_print("Match", 5);

                         if(plant_temperature_C < temperature_C){
                            UCB1I2CSA = 0x0069; Packet[0]=0xA; SetOnce=1;
                            Data_Cnt = 0;          //ensure count is zero
                            UCB1TBCNT = 1;         // set packet length to 1
                            UCB1CTLW0 |= UCTR;     // Transmitter mode
                            UCB1IE |= UCTXIE0;     // Enable TX interrupt
                            UCB1CTLW0 |= UCTXSTT;  // Start transmission
                            P4OUT &= ~(BIT2 | BIT3);
                         
                         while(plant_temperature_C < temperature_C){
                            //set to heat
                            P4OUT |= BIT2;
                            }
                            UCB1I2CSA = 0x0069; Packet[0]=0xD; SetOnce=1;
                            Data_Cnt = 0;          //ensure count is zero
                            UCB1TBCNT = 1;         // set packet length to 1
                            UCB1CTLW0 |= UCTR;     // Transmitter mode
                            UCB1IE |= UCTXIE0;     // Enable TX interrupt
                            UCB1CTLW0 |= UCTXSTT;  // Start transmission
                            P4OUT &= ~(BIT2 | BIT3);
                            LCD_clear_first_line(5);
                            break;
                         }
                        if(plant_temperature_C > temperature_C){
                            UCB1I2CSA = 0x0069; Packet[0]=0xB; SetOnce=1;
                            Data_Cnt = 0;          //ensure count is zero
                            UCB1TBCNT = 1;         // set packet length to 1
                            UCB1CTLW0 |= UCTR;     // Transmitter mode
                            UCB1IE |= UCTXIE0;     // Enable TX interrupt
                            UCB1CTLW0 |= UCTXSTT;  // Start transmission
                            P4OUT &= ~(BIT2 | BIT3);
                         
                         while(plant_temperature_C > temperature_C){
                            //set to cool     
                            P4OUT |= BIT3;
                            } 
                            UCB1I2CSA = 0x0069; Packet[0]=0xD; SetOnce=1;
                            Data_Cnt = 0;          //ensure count is zero
                            UCB1TBCNT = 1;         // set packet length to 1
                            UCB1CTLW0 |= UCTR;     // Transmitter mode
                            UCB1IE |= UCTXIE0;     // Enable TX interrupt
                            UCB1CTLW0 |= UCTXSTT;  // Start transmission
                            P4OUT &= ~(BIT2 | BIT3);
                            LCD_clear_first_line(5);
                            break;
                            }

                case 0xD: UCB1I2CSA = 0x0069; Packet[0]=0xD; SetOnce=1; 
                        Data_Cnt = 0;          //ensure count is zero
                        UCB1TBCNT = 1;         // set packet length to 1
                        UCB1CTLW0 |= UCTR;     // Transmitter mode
                        UCB1IE |= UCTXIE0;     // Enable TX interrupt
                        UCB1CTLW0 |= UCTXSTT;  // Start transmission 
                        LCD_clear_first_line(5);
                         for(i=0; i<100; i++){}  
                         rgb_control(2); __delay_cycles(500000); 
                         P4OUT &= ~(BIT3 | BIT2);           //heating and cooling off
                         break;

                default: 
                    break;
            }
        
            
    }

    return 0;
}

//------------------------------------------------------------------------------
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
//-- Timer_B ISR Triggers temperature reading every 0.5s----------------------
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B_ISR(void) {
   

//-- Plant temp conversation
    UCB1I2CSA = 0x48;                              // address 
    UCB1CTLW0 &= ~UCTR;                            // Receiver mode
    UCB1CTLW0 |= UCTXSTT;                          // Start
    plant_value = UCB1RXBUF;                       // Read one byte
    UCB1CTLW0 |= UCTXSTP;                          // Send STOP condition
//-- ADC conversation (call ISR)
    ADCCTL0 |= ADCENC | ADCSC;

    P6OUT ^= BIT6;
   
    TB0CCTL0 &= ~CCIFG;                            // Clear interrupt flag
}


/*ISR for reading ADC temperature, triggers every 0.5s from other ISR*/
#pragma vector = ADC_VECTOR
__interrupt void ADC_ISR(void)
{
                                // Read plant and ambient temperature
    adc_value = ADCMEM0;  


//-- ADC into moving average logic
   
    adc_sum -= adc_samples[sample_index];  // Subtract the oldest sample from sum
    
    adc_samples[sample_index] = adc_value; // Store new sample in array
 
    adc_sum += adc_value;                  // Add new sample to sum

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
        //break components of float into individual integers
        int whole = (int)temperature_C;
        tens = whole / 10;
        ones = whole % 10;
        decimal = (int)((temperature_C - whole) * 10);
        //Change these values to ascii chacters to be printed
        char temperature_string[6];
        temperature_string[0] = tens + '0';
        temperature_string[1] = ones + '0';
        temperature_string[2] = '.';
        temperature_string[3] = decimal + '0';
        temperature_string[4] = 223;       //ASCII code for degree symbol (in decimal)
        temperature_string[5] = 'C';
        LCD_command(0x8A);                      //move cursor to 11th space on first row
        LCD_print(temperature_string, 6);    
       
    
    }
//-- Plant into moving average logic (Equvalent to ADC logic above)


    plant_sum -= plant_samples[plant_index]; // Subtract the oldest sample from sum
  
    plant_samples[plant_index] = plant_value;// Store new sample in array

    plant_sum += plant_value;             // Add new sample to sum
    // Move to next index, wrap around if necessary
    plant_index = (plant_index + 1) % window_size;

    // Ensure we have enough samples before averaging
    if (plant_samples_collected < window_size) {
        plant_samples_collected++;
    }
    // Calculate rolling average temperature (once enough samples are collected)
    if (plant_samples_collected == window_size) {
        
        plant_temperature_C = (plant_sum/window_size);
        //break temperature into individual integer components
        int whole = (int)plant_temperature_C;
        tens = whole / 10;
        ones = whole % 10;
        decimal = (int)((plant_temperature_C - whole) * 10);
        //change these integers to ascii value to be printed
        char temperature_string[6];
        temperature_string[0] = tens + '0';
        temperature_string[1] = ones + '0';
        temperature_string[2] = '.';
        temperature_string[3] = decimal + '0';
        temperature_string[4] = 223;       //ASCII for degree symbol (in decimal)
        temperature_string[5] = 'C';
        LCD_command(0xCA);                      //move cursor to 11th space on the second row
        LCD_print(temperature_string, 6);    
       
       
    
    }
}
