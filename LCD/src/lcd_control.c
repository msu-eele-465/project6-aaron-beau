#include "lcd_control.h"
#include "intrinsics.h"
#include "sys/cdefs.h"



void LCD_init(){
    P1OUT &= ~BIT1;
    P1DIR |= BIT1;
//-----------------Ports Interfacing with LCD directly--------------------------------
    //Data Bus Pins (P1.7, 1.6, 1.5, 1.4)
    P1OUT &= ~(D7 | D6 | D5 | D4);
    P1DIR |= (D7 | D6 | D5 | D4);  
    //RS(P2.0) EN(P2.7)
    P2OUT &= ~(RS | EN);
    P2DIR |= (RS | EN);

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

   

}

/*
The LCD_setup function sets up the LCD screen to be in 4 bit mode by first setting the 
screen to be in 8 bit mode first.   This is accomplished by setting the upper nibble equal
to 0011b.  The screen is then set into four bit by setting the upper nibble to 0010b.  
The LCD command function is then used to set the screen to 2 line, with 5x8 characters
with the screen on, cursor off, not blinking, cleared, and with the cursor in the top 
left corner of the screen ready to display a message.
*/
void LCD_setup(){
    P2OUT &= ~RS;  // RS in command mode
__delay_cycles(50000);  // Power-up delay

// Force 8-bit mode (three times)
P1OUT &= ~(D7 | D6 | D5 | D4);
P1OUT |= 0b00110000;  
toggle_enable();
__delay_cycles(5000);

//Set 8 bit a second time
P1OUT &= ~(D7 | D6 | D5 | D4);
P1OUT |= 0b00110000;  
toggle_enable();
__delay_cycles(5000);

//Set 8 bit a third time
P1OUT &= ~(D7 | D6 | D5 | D4);
P1OUT |= 0b00110000;  
toggle_enable();
__delay_cycles(5000);

// Switch to 4-bit mode
P1OUT &= ~(D7 | D6 | D5 | D4);
P1OUT |= 0b00100000;  
toggle_enable();
__delay_cycles(000);


//Set display on with cursor blinking
P1OUT &= ~(D7 | D6 | D5 | D4);
P1OUT |= 0b00000000;
toggle_enable();
__delay_cycles(5000);

P1OUT &= ~(D7 | D6 | D5 | D4);
P1OUT |= 0b11000000;
toggle_enable();
__delay_cycles(30000);

//clear display
P1OUT &= ~(D7 | D6 | D5 | D4);
P1OUT |= 0b00000000;
toggle_enable();
__delay_cycles(5000);

P1OUT &= ~(D7 | D6 | D5 | D4);
P1OUT |= 0b00010000;
toggle_enable();
__delay_cycles(30000);

//Set increment with shift off
P1OUT &= ~(D7 | D6 | D5 | D4);
P1OUT |= 0b00000000;
toggle_enable();
__delay_cycles(5000);

P1OUT &= ~(D7 | D6 | D5 | D4);
P1OUT |= 0b01100000;
toggle_enable();
__delay_cycles(30000);

 
}
/*
The toggle_enable function is used to latch data being sent to the LCD screen
so that it can be read and acted upon.  This simply consists of setting the port
high, a brief delay, and then setting the port back to low.  This occurs anytime
a nibble of information is sent to the LCD
*/
void toggle_enable(){
   P2OUT |= EN;                // Set EN high
    __delay_cycles(500);         //  delay
    P2OUT &= ~EN;               // Set EN low
    __delay_cycles(500);         // delay
    
}
/*
The LCd_command function is used to send commands to the LCD screen.  Given that 
we are operating in 4 bit mode, we must send two nibble of data per command to the 
screen.  This is accomplished by taking in the command as an unsigned char, masking
the lower bit of the command and setting the data bus to that value.  The inverse is 
then done by making the upper bit and shifting it left so that it can be seen by the LCD
screen which is controlled by the top 4 bits of port 1.  Inbetween each nibble being sent
the toggle_enable function is called to latch the data so the LCD screen knows that a command
is being sent.
*/
void LCD_command(unsigned char cmd){
    P2OUT &= ~RS;               // RS = 0 (Command mode)
    P1OUT &= ~(D7 | D6 | D5 |D4);
    P1OUT |= (cmd);                // must lower nibble of Port 1 and cmd
    toggle_enable();                                        //Latch
    __delay_cycles(3000);  

    P1OUT &= ~(D7 | D6 | D5 | D4);
    P1OUT |= (cmd << 4);         //mask upper nibble, mask upper nibble of command and shift left
    toggle_enable();                                        //Latch
    __delay_cycles(3000);                                   //Delay for execution
    
} 

/*
The LCD_Clear function is a quick way to set the screen blank without
having to look at an arbitrary command number, however, it still utilizes
the LCD_command function.
*/
void LCD_Clear() {
    LCD_command(0b00000001);
    __delay_cycles(200);
   
}

/*
The LCD_write function will read in a binary value which correlates
to a character defined in the CGRAM table for the LCD.  The function
will then send the upper nibble of this binary value, latch, send the 
lower nibble, and latch again.  The LCD will process this value and
display the corresponding character
*/
void LCD_write(unsigned char message) {
    P2OUT |= RS;               // RS = 1 (Write mode)

    P1OUT &= ~(D7 | D6 | D5 | D4);
    P1OUT |= (message);                //  upper nibble of message
    toggle_enable();                                        //Latch
    __delay_cycles(3000);  

    P1OUT &= ~(D7 | D6 | D5 | D4);
    P1OUT |= (message << 4);         // shift message left 4 to send lower nibble
    toggle_enable();                                        //Latch
    __delay_cycles(3000);                                   //Delay for execution
}


/*
The LCD_clear_first_line function will simply set the cursor to the first 
character, print 16 blank spaces, then return back to the first character 
of the top line
*/
void LCD_clear_first_line(){
    int j;                          //loop variable
     LCD_command(0x80);             //set cursor to first character
     for(j = 0; j < 16; j++){
        LCD_write(0b00010000);      // blank character
     }
     LCD_command(0x80);             // reset cursor to first character
}   


/*
The LCD_print function will take in a predefined array corresponding to a
message that needs to be printed along with the length of the message (which inlcludes spaces).
The array will then be stepped through and each element printed to the LCD screen 
according to the codes defined in the CGRAM table for the LCD
*/
void LCD_print(const char *word, int length) {
    int i;
    for(i = 0; i < length; i++) {
        LCD_write(word[i]);  // Send each character to LCD
    }
}