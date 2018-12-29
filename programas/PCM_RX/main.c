#include <msp430.h> 

int valor;

void llamado(){
	 BCSCTL2 = SELM_0;
	BCSCTL1= CALBC1_16MHZ;
	DCOCTL= CALDCO_16MHZ;
	// se configura el reloj a 16 megaHZ
	P1SEL= BIT1 + BIT2;
	P1SEL2= BIT1+  BIT2;
	// se configura los p1.1 y P1.2 para servir como comunicacion serial...
}
void conf_serial(){
	UCA0CTL1 |= UCSSEL_2  +UCSWRST;// SMCLK
	UCA0BR0= 62; // VALOR QUE SE CARGA CON UNA FRECUENCIA 1 MHZ Y UNA MODULACION UCBRS0=1...
    UCA0BR1 = 0;
	UCA0MCTL= UCBRS_4; // MODULACION...
    //UCA0MCTL= UCBRF_3 + UCOS16; // MODULACION...
	UCA0CTL1 &= ~UCSWRST;
	IE2 = UCA0RXIE;  // SE ACTIVA LA INTERRUPCION POR RECEPCION.
	__enable_interrupt();
}

void main(void) {


WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    P2DIR =BIT0+BIT1+BIT2+BIT3+BIT4+BIT5+BIT6+BIT7;
    P2SEL=0;
    P2SEL2=0;
	llamado();
	conf_serial();
	P1DIR= BIT0;
	while(1){}
}
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
	P1OUT^=BIT0;
	valor= UCA0RXBUF;

	P2OUT = valor;
    IFG2&=~UCA0RXIFG;
}
