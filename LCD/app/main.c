
#include <msp430.h>
#include "src/lcd_control.h"

int RXDATA;
int plant_mode = 0xA;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    P1OUT &= ~BIT1;                         // Clear P1.1 output latch for a defined power-on state
    P1DIR |= BIT1;                          // Set P1.1 to output direction

    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings
    LCD_init();
    LCD_setup();

    while(1)
    {
        if(plant_mode == 0xA){
            LCD_clear_first_line(5);
            LCD_print("heat", 4);
            plant_mode = 0;
        }
        else if(plant_mode == 0xB){
            LCD_clear_first_line(5);
            LCD_print("cool", 4);
            plant_mode = 0;
        }else if(plant_mode == 0xC){
            LCD_clear_first_line(5);
            LCD_print("match", 5);
            plant_mode = 0;
        }else if(plant_mode == 0xD){
            LCD_clear_first_line(5);
            LCD_print("off", 3);
            plant_mode = 0;
        }

      
        P1OUT ^= BIT1;                      // Toggle P1.0 using exclusive-OR
        __delay_cycles(100000);             // Delay for 100000*(1/MCLK)=0.1s
    }
}

//--------- I2C Receive ISR (Handles Incoming Data) ---------------------------
/* ISR triggers upon start condition from I2C bus and receives sent data from
 * master 
 */
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_ISR(void)
{
    switch (__even_in_range(UCB0IV, 0x1E)) {
        case 0x16:  
            RXDATA = UCB0RXBUF;  // Read received byte

            if(RXDATA == 0xA || RXDATA == 0xB || RXDATA == 0xC || plant_mode == 0xD){       //check to see if user mode has been selected
                plant_mode = RXDATA;                   // set transmission to select user mode
            }
            break;

        case 0x12:  // UCSTPIFG: Stop condition detected
            UCB0IFG &= ~UCSTPIFG;  // Clear STOP flag
            break;

        default:
            break;
    
    
    }
}