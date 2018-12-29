#include <msp430.h> 
#include "QmathLib.h"
#include <stdint.h>
/*
 * main.c
 */
_q12 X, Y, Z;
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	

    X = _Q12(1.0);
   // Y = _Q12(7.0);
    //Z = _Q12div(X, Y);

    return 0;
}
