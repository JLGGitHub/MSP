#include <msp430.h> 
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "QmathLib.h"

_q12 X,  Y;
_q12 P;
main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	X = _Q12(1.0);
	Y = _Q12(7.0);
	P = _Q12div(X, Y);

}
