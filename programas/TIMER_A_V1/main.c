#include  <msp430g2553.h>
volatile int  valor;
void config(){
	//BCSCTL2 = SELM_0;
	BCSCTL1= CALDCO_1MHZ;
	DCOCTL=CALDCO_1MHZ;
	TA0CCTL0= CCIE;  // ACTIVA LA INTERRUPCION POR COMPARACION Y CAPTURA...
	TA0CTL= TASSEL_2 + MC_3 + ID_3;
	_BIS_SR(GIE);
	P1DIR= BIT0+BIT6;
	CCR0= 65500;
	P1OUT=~BIT6;
}
void spi(){
	P1SEL  =   BIT1    |   BIT2    |   BIT4;
	P1SEL2 =   BIT1    |   BIT2    |   BIT4;
	P2DIR= BIT0+BIT1;
	P2OUT= 0x03;
	UCA0CTL1   =   UCSWRST;
	UCA0CTL0   |=   UCMSB   +   UCMST   +   UCSYNC; //  3-pin,  8-bit   SPI master
	UCA0CTL1   |=  UCSSEL_2;   //  SMCLK
	UCA0BR0    |=  0x02;   //  /2
	UCA0BR1    =   0;  //
	UCA0MCTL   =   0;  //  No  modulation
	UCA0CTL1   &=  ~UCSWRST;   //  **Initialize    USCI    state   machine**ç
}
void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	config();
	spi();
	while(1){}
}

#pragma vector= TIMER0_A0_VECTOR
__interrupt void timer_A (void){
	P1OUT^=BIT6;
	valor+=50;
	if(valor>251){valor=0;}
	 while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
	  		 UCA0TXBUF=valor>>4;
	  		 while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
	  		 UCA0TXBUF=valor<<4;

	  		 	 P2OUT= 0x02; // apago bit 0
	  		 	 P2OUT= 0x01; // apage bit 1
	  		 	 P2OUT= 0x03; // prendi bit 0
	  		 	 P2OUT= 0x03; // prendi bit 1
	  		 	TA0CCTL0 &= ~CCIFG;
}
