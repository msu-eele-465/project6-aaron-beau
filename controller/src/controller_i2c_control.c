#include "src/controller_control.h"
#include "intrinsics.h"
#include "sys/cdefs.h"

void controller_i2c_init(){
//------------------------------- I2C Initialization -----------------------------    
    UCB1CTLW0 |= UCSWRST;                  // UCSWRST =1 for eUSCI_B1 in SW reset
	
//--Configure eUSCI_B1
    UCB1CTLW0 |= UCSSEL_3;                 // Choose BRCLK=SMCLK=1MHz
    UCB1BRW = 10;                          // Divide BRCLK by 10 for SCL=100kHz

    UCB1CTLW0 |= UCMODE_3;                 // Put into I2C mode
    UCB1CTLW0 |= UCMST;                    // Put into master mode
	UCB1CTLW0 |= UCTR;                     // Put into Tx mode
	//UCB1I2CSA = 0x0069;                    // Slave address = 0x68

    UCB1CTLW1 |= UCASTP_2;                 // Auto STOP when UCB1TBCNT reached
	UCB1TBCNT = 1;            // # of Bytes in Packet

//--Configure Ports SDA (P4.6) and SCL (P4.7)
    P4SEL1 &= ~(BIT6 | BIT7);
    P4SEL0 |= (BIT6 | BIT7);

//--Take eUSCI_B1 out of SW reset
    UCB1CTLW0 &= ~UCSWRST;

//--Enable Interrupts AFTER exiting reset
	UCB1IE |= UCTXIE0;                     // Enable I2C Tx0 IRQ
    __enable_interrupt();                  // Enable Maskable IRQs

}
