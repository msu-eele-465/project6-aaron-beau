#include "src/controller_control.h"
#include "intrinsics.h"
#include "sys/cdefs.h"

void controller_init(){
//----------------------------Pin Initialization--------------------------------
//--RGB LED (1=red, 2=green, 3=blue, 4=yellow)
    P4DIR |= BIT0;                              // Configure red part
    P4OUT &= ~BIT0;

    P2DIR |= BIT2;                              // Configure green part
    P2OUT &= ~BIT2;

    P2DIR |= BIT0;                              // Configure blue part
    P2OUT &= ~BIT0;

//--Columns
    P3DIR |= BIT4 | BIT5 | BIT6 | BIT7;        // Set columns as outputs
    P3OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);     // Initially set all low

//--Rows
    P3DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3);
    P3REN |= BIT0 | BIT1 | BIT2 | BIT3;
    P3OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);

    P6DIR |= BIT6;                             // Set P16 to output direction
    P6OUT |= BIT6;      // Clear P16 output latch for a defined power-on state

//--Thermo Plant Pins (P4.2 = Heat and P4.3 = Cool)
    P4OUT &= ~(BIT2 | BIT3);
    P4DIR |= (BIT2 | BIT3);


   
//----------------------------Timer_B Initialization----------------------------
    TB0CCTL0 |= CCIE;         // Enable Timer_B interrupt
    TB0CCR0 |= 62500;         // 125kHz / 2 = 0.5s interval
    TB0CTL |= TBSSEL_2;      // Select SMCLK (1 MHz) 
    TB0CTL |= MC_1;          // Set Timer_B to Up Mode 
    TB0CTL |= ID_3;          // Set input divider to 8 
    TB0CTL |= TBCLR;         // Clear Timer
   
//------------------------------- I2C Initialization -----------------------------    
    UCB1CTLW0 |= UCSWRST;                      // UCSWRST =1 for eUSCI_B1 in SW reset
	
//--Configure eUSCI_B1
    UCB1CTLW0 |= UCSSEL_3;                     // Choose BRCLK=SMCLK=1MHz
    UCB1BRW = 10;                              // Divide BRCLK by 10 for SCL=100kHz

    UCB1CTLW0 |= UCMODE_3;                     // Put into I2C mode
    UCB1CTLW0 |= UCMST;                        // Put into master mode
	UCB1CTLW0 |= UCTR;                         // Put into Tx mode
	//UCB1I2CSA = 0x0069;                      // Slave address = 0x68

    UCB1CTLW1 |= UCASTP_2;                     // Auto STOP when UCB1TBCNT reached
	UCB1TBCNT = 1;                             // # of Bytes in Packet

//--Configure Ports SDA (P4.6) and SCL (P4.7)
    P4SEL1 &= ~(BIT6 | BIT7);
    P4SEL0 |= (BIT6 | BIT7);

//--Take eUSCI_B1 out of SW reset
    UCB1CTLW0 &= ~UCSWRST;

//--Enable Interrupts AFTER exiting reset
	UCB1IE |= UCTXIE0;                         // Enable I2C Tx0 IRQ
    __enable_interrupt();                      // Enable Maskable IRQs
}




