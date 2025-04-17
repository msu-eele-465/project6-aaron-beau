/*
This module contains all of the pattern information that is driven to the off board LED bar display.
An integer denoting which pattern is desired is passed into the function and will then be driven 
through the timer ISR within the main loop of the system.  A count flag is implemented so that when a 
pattern is changed it can return to the place where it was before the pattern was changed.  All ports are
directly driven to the off board LED bar display
*/

#include "lightbar.h"
#include "intrinsics.h"
#include "msp430fr2310.h"
#include <stdint.h>

int lightbar(int count, int patt, uint8_t value){        //function to carry through each pattern

 if(patt == 0xA) {
    
    // System is actively heating
    if (count == 0) {
        P1OUT |= BIT4; // Light 3
        count++;
    } else if (count == 1) {
        P1OUT |= BIT5; // Light 4
        count++;
    } else if (count == 2) {
        P1OUT |= BIT6; // Light 5
        count++;
    } else if (count == 3) {
        P1OUT |= BIT7; // Light 6
        count++;
    } else if (count == 4) {
        P2OUT |= BIT0; // Light 7
        count++;
    } else if (count == 5) {
        P2OUT |= BIT6; // Light 8
        count++;
    } else if (count == 6) {
        P2OUT |= BIT7; // Light 9
        count++;
    } else if (count == 7) {
        P1OUT |= BIT0; // Light 10
        count=0;
    }
}
if(patt == 0xB){
    if (count == 0){
        P1OUT |= BIT0;           // Light 3
        count++;
    } else if (count == 1){
        P2OUT |= BIT7;           // Light 4
        count++;
    } else if (count == 2){
        P2OUT |= BIT6;           // Light 5
        count++;
    } else if (count == 3){
        P2OUT |= BIT0;           // Light 6
        count++;
    } else if (count == 4){
        P1OUT |= BIT7;           // Light 7
        count++;
    } else if (count == 5){
        P1OUT |= BIT6;           // Light 8
        count++;
    } else if (count == 6){
        P1OUT |= BIT5;           // Light 9
        count++;
    } else if (count == 7){
        P1OUT |= BIT4;           // Light 10
        count=0;
    } 
}
    return count;
}
