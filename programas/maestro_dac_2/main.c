#include    <msp430.h>
 int valor;
volatile    char    received_ch =   0;
int main(void)
{
	P1DIR=BIT6;
	BCSCTL1 = CALBC1_1MHZ;    //configurando
    DCOCTL = CALDCO_1MHZ;     //DCO 16Mhz
	WDTCTL =   WDTPW   +   WDTHOLD;    //  Stop    WDT
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
	valor=150;

while(1){}
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void)
{
	P1OUT^= BIT6;
	TA0CCR0+=600;
	//valor= 60;  // valor qeu se carga para comprobar que funcione el codigo...
		ADC10CTL0 |=  ADC10SC;
		//while(ADC10CTL1 & ADC10BUSY); //esperamos que este lista la conversion
		//valor = ADC10MEM>>2;                       // guarda en una variable

	    while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
  		 UCA0TXBUF=valor>>4;
  		 while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
  		 UCA0TXBUF=valor<<4;

  		 	 P2OUT= 0x02; // apago bit 0
  		 	 P2OUT= 0x01; // apage bit 1
  		 	 P2OUT= 0x03; // prendi bit 0
  		 	 P2OUT= 0x03; // prendi bit 1


  		  TA0CCTL0 &= ~CCIFG;
