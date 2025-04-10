
#include <msp430.h>
#include "src/lcd_control.h"


int RXDATA;

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

        LCD_print("Hello World", 11);
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

                
       
            break;

        case 0x12:  // UCSTPIFG: Stop condition detected
            UCB0IFG &= ~UCSTPIFG;  // Clear STOP flag
            break;

        default:
            break;
    }
    
    }
}