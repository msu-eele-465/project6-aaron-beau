/*----------------------------------------------------------------------------------
Beau Coburn & Aaron Foster
EELE 465
2/25/25
Project 3: Keypad LED Pattersn
Description:  




-------------------------------------------------------------------------------------*/
#include <msp430.h> 
#include "plant_temp.h"
#include "intrinsics.h"
#include "msp430fr2355.h"
#include <stdint.h>

 char register0[] = {0x00};                         // Tx Packet


void plant_temp_init(){
    // Set the slave address
    UCB1I2CSA = 0x48; // address 

    // Send the register pointer
    UCB1CTLW0 |= UCTR;                             // Tx mode
    UCB1CTLW0 |= UCTXSTT;                          // Start

    UCB1TXBUF = 0x00;                              // Register pointer

}


