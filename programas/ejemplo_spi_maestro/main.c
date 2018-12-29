#include    <msp430.h>
volatile int x ,dato;
volatile    char    received_ch =   0;

int main(void)
{ P1DIR=BIT6;
DCOCTL = DCO0 + DCO1;
BCSCTL1 = RSEL0; // SE SELECCIONA UNA FR
CCTL0 = CCIE;                             // CCR0 interrupt HABILITADA
CCR0 = 18750;
TACTL = TASSEL_2 + MC_1 + ID_3; //
	WDTCTL =   WDTPW   +   WDTHOLD;    //  Stop    WDT
	P1SEL  =   BIT7       |   BIT5 |   BIT1 |   BIT2;
	P1SEL2 =   BIT7       |   BIT5 |   BIT1 |   BIT2;

	UCB0CTL1   =   UCSWRST;
	UCB0CTL0   |=  UCCKPH  +   UCMSB   +   UCMST   +   UCSYNC; //  3-pin,  8-bit   SPI master
	UCB0CTL1   |=  UCSSEL_2;   //  SMCLK
	UCB0BR0    |=  0x02;   //  /2
	UCB0BR1    =   0;  //
	//UCB0MCTL   =   0;  //  No  modulation
	UCB0CTL1   &=  ~UCSWRST;   //  **Initialize    USCI    state   machine**ç

	UCA0CTL1= UCSSEL_2; // SMCLK
	UCA0BR0= 104; // VALOR QUE SE CARGA CON UNA FRECUENCIA 1 MHZ Y UNA MODULACION UCBRS0=1...
    UCA0BR1 = 0;
	UCA0MCTL= UCBRS0; // MODULACION...
	UCA0CTL1 &= ~UCSWRST;
	IE2 = UCA0RXIE;  // SE ACTIVA LA INTERRUPCION POR RECEPCION.
	_BIS_SR(GIE);  // INTERRUPCIONES GLOBALES


while(1){
	while  (!(IFG2 &   UCB0TXIFG));    //  USCI_A0 TX  buffer  ready?
		UCB0TXBUF  =   x;  }
}

/*#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  P1OUT ^= BIT6;                            // Toggle P1.0
  x++;
  	if(x>7){x=0; }
}*/

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
	dato= UCA0RXBUF;
	IFG2&=~UCA0RXIFG;
	P1OUT^=BIT6;

	switch(dato){
				case 'a': {x=1;

				} break;
				case 'b': {x=0;

							} break;

		}


