/*-----------------------------------------------------------------------------
* Project 6: Multi MSP with LED bar and LCD Display and Temp sensor
* Aaron Foster & Beau Coburn
* EELE 465
* 3/25/25
* 
* Main file for the Led-bar peripheral msp. receives commands from master via
* I2C and changes LED pattern accordingly.
*///----------------------------------------------------------------------------
#include <msp430.h>
#include "intrinsics.h"
#include "src/lightbar.h"
#include "msp430fr2310.h"
#include <stdint.h>

#define SLAVE_ADDR 0x69                     // Define I2C slave address

//-------------------Variable Initialization -----------------------------------
/* Many variables are for counting and control logic, see descriptions below */
//-----------------------------------------------------------------------------
static int stepnum = 0;                     // Current step in lightbar pattern
static int barflag = 0;                     // Flag to update lightbar state
static int pattspec = 0;                    // Specifies the lightbar pattern

int pattnum1 = 0;                           // Pattern counter 1
int pattnum3 = 0;                           // Pattern counter 3

int temp = 0;                               // Temporary storage for pattern
volatile static uint8_t lightbar_byte = 0;  // Stores lightbar data (volatile for ISR)
static int time_cntl = 0;                   // Controls pattern timing
static int base_time=3;                       // Base time for pattern timing
int barcounter = 0;                         // Counter for bar updates
static int status = 10;                     // Status indicator
volatile int wait;

//---------------------- I2C Variables ----------------------
volatile uint8_t Received = 0;                  // Single-byte storage for I2C reception
//-----------------------------------------------------------
#include <msp430fr2310.h>
#include <stdbool.h>

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

//------------------------------------------------------------------------------
//---------------------------- Pin Initialization ------------------------------
//------------------------------------------------------------------------------

//------Setup Timer B0 (0.25s interval for heartbeat and patterns)
    TB0CTL |= TBCLR;                        // Clear timer
    TB0CTL |= TBSSEL__SMCLK;                // Select SMCLK (1 MHz)
    TB0CTL |= MC__UP;                       // Set mode to "up"
    TB0CTL |= ID__4;                        // Divide clock by 4
    TB0CCR0 = 65500;                        // Set overflow period to ~0.25s
    TB0CTL |= TBIE;                         // Enable Timer Overflow Interrupt
    TB0CTL &= ~TBIFG;                       // Clear pending interrupt flag
    __enable_interrupt();                   // Enable global interrupts

//------------------------------- I2C Initialization -----------------------------    
//--Put eUSCI_B0 into software reset to allow configuration
    UCB0CTLW0 |= UCSWRST;       

//--Configure eUSCI_B0 for I2C Slave mode
    UCB0CTLW0 |= UCMODE_3 | UCSYNC;         // Set I2C mode, synchronous operation
    UCB0I2COA0 = SLAVE_ADDR | UCOAEN;       // Set slave address & enable

//--Configure Ports for I2C SDA (P1.2) and SCL (P1.3)
    P1SEL1 &= ~(BIT3 | BIT2);               // Select primary module function for I2C
    P1SEL0 |= (BIT3 | BIT2);

//--Enable I2C Module
    UCB0CTLW0 &= ~UCSWRST;                  // Release eUSCI_B0 from reset

//--Enable I2C Interrupts
    UCB0IE |= UCRXIE0;                      // Enable I2C receive interrupt
    __bis_SR_register(GIE);                 // Enable global interrupts

//----- 10-Segment Display Initialization and Status LED (P1.1) ------------
    P1DIR |= (BIT0 | BIT1 | BIT7 | BIT6 | BIT5 | BIT4); 
    P1OUT &= ~(BIT0 | BIT1 | BIT7 | BIT6 | BIT5 | BIT4);

    P2DIR |= (BIT7 | BIT6 | BIT0);
    P2OUT &= ~(BIT7 | BIT6 | BIT0);

//------------------------------------------------------------------------------
//------------------------ End Pin Initialization ------------------------------
//------------------------------------------------------------------------------

    PM5CTL0 &= ~LOCKLPM5;                  // Disable GPIO high-impedance mode
    pattspec=2;                            // Testing
    while (1){
//--Timer LED Logic
        if (status != 0) {                 // If timer countdown is active
            P1OUT |= BIT1;                 // Turn on status LED
        } else {
            P1OUT &= ~BIT1;                // Turn off status LED
        }


//--Lightbar Pattern Update Logic

        if (barflag) {
            barflag = 0;           // Reset flag to avoid immediate retrigger
            switch (pattspec) {
                
                case 0xA:                    // Heating
                    stepnum = lightbar(stepnum, pattspec, lightbar_byte);
                    temp = pattspec;
                    if(stepnum==0){
                        pattspec=0;
                    }
                    break;
                case 0xB:                    // Cooling
                    stepnum = lightbar(stepnum, pattspec, lightbar_byte);
                    temp = pattspec;
                    if(stepnum==0){
                        pattspec=0;
                    }
                case 0xD:                    // Turn off all LEDs
                    P1OUT &= ~(BIT0 | BIT7 | BIT6 | BIT5 | BIT4); 
                    P2OUT &= ~(BIT7 | BIT6 | BIT0);                
                    break;

                default:    
                    break;
                }
            }
           
    }
}

//------------------------------------------------------------------------------
//---------------------- Interrupt Service Routines (ISRs) ---------------------
//------------------------------------------------------------------------------

//--------------- Timer B0 Overflow ISR (Handles Lightbar Timing) --------------
/* ISR counts every 0.25s for led bar and can be adjusted for time modulation 
 * inputs from keypad
 */
#pragma vector = TIMER0_B1_VECTOR
__interrupt void ISR_TB0_OVERFLOW(void)
{
    if (status != 0) {
        status--;                         // Decrease countdown timer
    }
    barcounter++;                         // Increment lightbar timing counter
    if (barcounter >= (base_time + time_cntl)) {
        barcounter = 0;                   // Reset counter
        barflag = 1;                      // Trigger lightbar update 
        if (pattspec == 3) {  
            if (++lightbar_byte > 255) lightbar_byte = 0;
        } else if (stepnum > 7 && pattspec != 2) {
            barflag = 0;
            stepnum = 0;
        }
    }
    TB0CTL &= ~TBIFG;                     // Clear interrupt flag
}

//--------- I2C Receive ISR (Handles Incoming Data) ---------------------------
/* ISR triggers upon start condition from I2C bus and receives sent data from
 * master 
 */
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_ISR(void)
{
    switch (__even_in_range(UCB0IV, 0x1E)) {
        case 0x16:                        // UCRXIFG0: Byte received
            status = 10;
            Received = UCB0RXBUF;         // Read received byte
                                          // Force an ACK manually and
            UCB0CTLW0 &= ~UCTXACK;        // Ensure ACK is sent
            if(Received == 0xB){
                wait = 1;
            }
            pattspec=Received;
            
        case 0x12:                        // UCSTPIFG: Stop condition detected
            UCB0IFG &= ~UCSTPIFG;         // Clear STOP flag
            break;

        default:
            break;
    P1OUT &= ~BIT0;
    P1DIR |= BIT0;

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    while (true)
    {
        P1OUT ^= BIT0;

        // Delay for 100000*(1/MCLK)=0.1s
        __delay_cycles(100000);

    }
}

}


