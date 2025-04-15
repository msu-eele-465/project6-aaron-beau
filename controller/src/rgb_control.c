/*
This module contains all of the colors used to distinguish between locked, unlocking, unlocked,
and the various states of the LED.  A simple integer needs to be passed into the function for a color
to be selected and driven to the off board RGB LED
*/

#include "rgb_control.h"
#include "intrinsics.h"

void rgb_control(int color){        //1=red, 2=green, 3=blue, 4=yellow

    if(color == 1){             //red
        P4OUT |= BIT0;
        P2OUT &= ~BIT2;
        P2OUT &= ~BIT0;

    }else if (color == 2) {     //green     
        P4OUT &= ~BIT0;
        P2OUT |= BIT2;
        P2OUT &= ~BIT0;

    }else if (color == 3) {     //blue
        P4OUT &= ~BIT0;
        P2OUT &= ~BIT2;
        P2OUT |= BIT0;
    }else if (color ==4) {      //yellow
        P4OUT |= BIT0;
        P2OUT |= BIT2;
        P2OUT &= ~BIT0;

        P4OUT |= BIT0;
        P2OUT |= BIT2;
    }else if(color == 5){       //light purple
        P4OUT |= BIT0;
        P2OUT |= BIT2;
        P2OUT &= ~BIT0;
    }else if(color == 6){       //dark purple
        P2OUT |= BIT0;
        P2OUT |= BIT2;
        P4OUT |= BIT0;
    }else if(color ==7){
         P2OUT |= (BIT2 | BIT0);  // Green + Blue
         P4OUT &= ~BIT0;
    }
}
