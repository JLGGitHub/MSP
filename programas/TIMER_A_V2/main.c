#include <msp430g2553.h>

void main(void)
{

	BCSCTL1= CALBC1_1MHZ;
	DCOCTL= CALDCO_1MHZ;
P1OUT=0x00;
WDTCTL = WDTPW + WDTHOLD;                 // detiene el wdt
P1DIR |= BIT6+BIT0;                            //
CCTL0 = CCIE;                             // CCR0 interrupt HABILITADA
CCR0 = 500;
TACTL = TASSEL_2 + MC_1; //
_BIS_SR(GIE);                 // Enter LPM0 w/ interrupt

	while(1);
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  P1OUT ^= BIT6;                            // Toggle P1.0
}
