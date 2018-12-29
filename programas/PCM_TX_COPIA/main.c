#include <msp430G2553.h>
//#include "mensaje_Serial.h"
unsigned int valor;

void adc(){
	ADC10CTL0= SREF_0 + ADC10SHT_2 + ADC10ON;
		ADC10CTL1= INCH_5 + SHS_0 + ADC10DIV_0 + ADC10SSEL_0 + CONSEQ_0;
		ADC10AE0= BIT5;
		ADC10CTL0 |= ENC;

}
void muestreo(){
	TA0CTL = TASSEL_2  + ID_0 + MC_2;
	TA0CCR0 = 700;                     //estaba con 1000
	TA0CCTL0= CCIE;   // interrupcion habilitada
	P1DIR |= BIT6;
}


void serial(){
			P1SEL= BIT1 + BIT2;
			P1SEL2= BIT1+  BIT2;// se configura los p1.1 y P1.2 para servir como comunicacion serial...
			UCA0CTL1 |= UCSSEL_2  +UCSWRST;// SMCLK
			UCA0BR1 = 0;
			UCA0BR0= 62; // VALOR QUE SE CARGA CON UNA FRECUENCIA 16 MHZ Y UNA MODULACION UCBRS0=1...
		    UCA0MCTL= UCBRS_4;  // MODULACION...
			UCA0CTL1 &= ~UCSWRST;


}
void main(void) {

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
		 BCSCTL2 = SELM_0;          //MCLK=SMCLK=DCO
		BCSCTL1= CALBC1_16MHZ;
		DCOCTL= CALDCO_16MHZ;// se configura el reloj a 16 megaHZ
		adc();
		muestreo();
		serial();

	__enable_interrupt();
	
	while(1){
	}
}
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void)
{
	P1OUT^= BIT6;
	TA0CCR0+=700;
			ADC10CTL0 |=  ADC10SC;
			while(ADC10CTL1 & ADC10BUSY); //esperamos que este lista la conversion
		valor = ADC10MEM>>2;                       // guarda en una variable
	    while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
  		 UCA0TXBUF=valor;
  		  //TA0CCTL0 &= ~CCIFG;

}


