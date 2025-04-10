/*----------------------------------------------------------------------------------
Beau Coburn & Aaron Foster
EELE 465
2/25/25
Project 3: Keypad LED Pattersn
Description:  

scan_keypad
    This function scans the input of the key pad by setting the rows as inputs and the columns as outputs.
    The first column is set high and all the rows are checked to see if both the row and column are high corresponding
    to a key press.  If no key is detected on the first column, the logic will continue and the next row set high
    till a key press is detected.  A value will only be returned if a key is pressed and will only be sent when the
    key is released.  

unlock_keypad
    The above keypad logic is used to read in four digits from the keypad.  With those digits loaded into an array
    they are compared to the set code.  If the entered code mathech the set code the locked flag will be set zero and 
    returned to the main C file to unlock the sytem.  If the code entered is incorrect the system will stay locked.
    while the 4 entered values are being loaded into the array the RGB LED is set to yellow to denote that the
    system is attempting to be unlocked.

led_pattern
    The scan_keypad logic is used to scan in a single digit input from the keypad.  When a key is pressed
    it will be returned to the main code which will use it to set the desired LED pattern on the off board
    LED bar display.  This functionality is also used to return the input (A or B) which changes the speed
    at which the LED pattern is displayed.  Additionaly, this is used to detect when D is pressed to reset the sytem.
    Logic was used to decode the ascii character to a decimal value so that the desired value was valid within the LED
    pattern module

Pin Out
--------------
Rows : P3.0, P3.1, P3.2, P3.3 
Cols : P3.4, P3.5, P3.6, P3.7

//This line was added to fix git pushing
-------------------------------------------------------------------------------------*/
#include <msp430.h> 
#include "rgb_control.h"
 
// Keypad mapping
char keymap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

char scan_keypad(void){
    unsigned int row, col;
    const char row_pin[4] = {BIT0, BIT1, BIT2, BIT3};
    const char col_pin[4] = {BIT4, BIT5, BIT6, BIT7};

    // Loop through all the columns
    for(col = 0; col < 4; col++) {
        // Set all columns low
        P3OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);  // Set all columns low
        
        // Set the current column to HIGH
        P3OUT |= col_pin[col];                             // Set the current column high

        // Check each row to see if any row is low (indicating a key press)
        for(row = 0; row < 4; row++) {
            if((P3IN & (row_pin[row])) != 0) {               // If the row is pulled low
            //P1OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);  // Set all columns low
                // Wait for key release to avoid multiple detections
                while((P3IN & (row_pin[row])) != 0);
                return keymap[row][col];                    // Return the key from the keymap
            }
        }
    }

    return 0; // No key pressed

}

int unlock_keypad(void){ 
    int i = 0;
    int equal = 1;
    int locked = 1;
    char key;                   
    char unlock_code[4];        
    char set_code[4] = {'6','9','6','9'};             
         // Collecting 4-digit unlock code
        i = 0;                                  // Reset index before input
        while (i < 4 && locked == 1) {
            key = scan_keypad();
            if (key != 0) {                     // Only store valid key presses
            rgb_control(4);
                unlock_code[i] = key;
                i++;
            }
        }

        // Check if entered code matches set_code
        
        for (i = 0; i < 4; i++) {
            if (unlock_code[i] != set_code[i]) {
                equal = 0;                      // Codes do not match
                break;
            }
        }

        if (equal == 1) {
            locked = 0;                         // Unlock system
            
        } else {
            locked = 1;                         // Keep system locked
            
        }
        return locked;
    }

    char led_pattern(void){
       static int last_patt = 0;  // Stores the last selected pattern, initially 0
    char key = 0;

    while (key == 0) {  // Wait for a new key press
        key = scan_keypad();
    }

    // Convert key from ASCII to integer
    if (key >= '0' && key <= '9') {
        last_patt = key - '0';
    } else {
        switch (key) {
            case 'A': last_patt = 0xA; break;
            case 'B': last_patt = 0xB; break;
            case 'C': last_patt = 0xC; break;
            case 'D': 
                last_patt = 0;  // Reset pattern when locking
                return 0xD;  // Immediately return 13 to signal relock
            case '*': last_patt = 14; break;
            case '#': last_patt = 15; break;
            default: break;  // Ignore invalid keys
        }
    }

    return last_patt;
    }