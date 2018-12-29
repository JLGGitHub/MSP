#include <msp430G2553.h>
//#include "mensaje_Serial.h"
unsigned int valor;
int comando ;
volatile char dato;
volatile int m;
//#define ldac  BIT0
//#define load BIT1
//#define comando 0x00;
void adc(){
	ADC10CTL0= SREF_0 + ADC10SHT_2 + ADC10ON;
		ADC10CTL1= INCH_4 + SHS_0 + ADC10DIV_0 + ADC10SSEL_0 + CONSEQ_0;
		ADC10AE0= BIT4;
		ADC10CTL0 |= ENC;

}
void muestreo(){
	TA0CTL = TASSEL_2  + ID_0 + MC_2;
	TA0CCTL0= CCIE;   // interrupcion habilitada
	P1DIR |= BIT6;
	P1DIR |= BIT0;
}

void spi(){
	P1SEL  =   BIT1    |   BIT2    | BIT5  | BIT7;
		P1SEL2 =   BIT1    |   BIT2    |  BIT5  | BIT7 ;

		UCB0CTL1   =   UCSWRST;
		UCB0CTL0   |=  UCMSB   +   UCMST   +   UCSYNC; //  3-pin,  8-bit   SPI master
		UCB0CTL1   |=  UCSSEL_2;   //  SMCLK
		UCB0BR0    |=  0x02;   //  /2
		UCB0BR1    =   0;  //
		//UCA0MCTL   =   0;  //  No  modulation
		UCB0CTL1   &=  ~UCSWRST;   //  **Initialize    USCI    state   machine**ç

			UCA0CTL1= UCSSEL_2+UCSWRST; // SMCLK
			UCA0BR0= 130; // VALOR QUE SE CARGA CON UNA FRECUENCIA 1 MHZ Y UNA MODULACION UCBRS0=1...
		    UCA0BR1 = 6;
			UCA0MCTL= UCBRS_6; // MODULACION...
			UCA0CTL1 &= ~UCSWRST;
			IE2 = UCA0RXIE;  // SE ACTIVA LA INTERRUPCION POR RECEPCION.

}
void main(void) {
		P2DIR= BIT0+BIT1;
		P2OUT= 0x03;
		WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
		 BCSCTL2 = SELM_0;          //MCLK=SMCLK=DCO
		BCSCTL1= CALBC1_16MHZ;
		DCOCTL= CALDCO_16MHZ;// se configura el reloj a 16 megaHZ
		adc();
		muestreo();
		spi();
		__enable_interrupt();

	while(1){
	}
}
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void)
{
	P1OUT^= BIT6;
	TA0CCR0+=m;
	//valor= 60;  // valor qeu se carga para comprobar que funcione el codigo...
		ADC10CTL0 |=  ADC10SC;
		while(ADC10CTL1 & ADC10BUSY); //esperamos que este lista la conversion
		valor = ADC10MEM>>2;                       // guarda en una variable

	    while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
  		 UCB0TXBUF=valor>>4;
  		 while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
  		 UCB0TXBUF=valor<<4;

  		 	 P2OUT= 0x02; // apago bit 0
  		 	 P2OUT= 0x01; // apage bit 1
  		 	 P2OUT= 0x03; // prendi bit 0
  		 	 P2OUT= 0x03; // prendi bit 1
  		 	 TA0CCTL0 &= ~CCIFG;
}
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
	dato= UCA0RXBUF;
	IFG2&=~UCA0RXIFG;
	P1OUT^=BIT0;
	switch(dato){
					case 'a': {
						m=32345;  // aliasing
					} break;
					case 's': {  // 2bw
						m=6000;   //8k
					} break;
					case 'd':{ // 8bw
						m=2500;  //2k
					} break;
					case 'f':{  //10bw
						m=1600;
					} break;
					case 'g':{  // sobremuestreo
						m=700;
					} break;
				}

		}
