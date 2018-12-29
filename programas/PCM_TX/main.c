#include <msp430G2553.h>
//#include "mensaje_Serial.h"
unsigned int valor;

void adc(){

	ADC10AE0 |= BIT0;        //enable
		ADC10CTL0 = SREF_0 + ADC10SHT_0 + ADC10ON; // 4ADC10CLKs VSS Y VCC referencia
		ADC10CTL1 = INCH_0 + SHS_0 + ADC10DIV_0 + ADC10SSEL_0 + CONSEQ_0;
}
void muestreo(){

	TA0CTL = TASSEL_2  + ID_0 + MC_2;   //fuente SM dividido en 1 continuo
	TA0CCR0 = 700;   // cuenta estas ciclos
	TA0CCTL0= CCIE;   // interrupcion habilitada

}

void llamado(){
	P1SEL= BIT1 + BIT2;
	P1SEL2= BIT1+  BIT2;// se configura los p1.1 y P1.2 para servir como comunicacion serial...
}
void conf_serial(){
	UCA0CTL1 |= UCSSEL_2  +UCSWRST;// SMCLK
	UCA0BR0= 130; // VALOR QUE SE CARGA CON UNA FRECUENCIA 1 MHZ Y UNA MODULACION UCBRS0=1...
    UCA0BR1 = 6;
    UCA0MCTL= UCBRS_6;  // MODULACION...
	UCA0CTL1 &= ~UCSWRST;
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	 BCSCTL2 = SELM_0;          //MCLK=SMCLK=DCO
	BCSCTL1= CALBC1_16MHZ;
	DCOCTL= CALDCO_16MHZ;// se configura el reloj a 16 megaHZ
    muestreo();
	llamado();
	adc();
	conf_serial();
	P1DIR |= BIT6;
	__enable_interrupt();
	
	while(1){
	}
}
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void)
{
	TA0CCR0+=700;
	P1OUT^= BIT6;
		ADC10CTL0 |= ENC + ADC10SC;           // inicia conversion
		while(ADC10CTL0 & ADC10SC);           // espera a que termine SC
		ADC10CTL0 &= ~ENC;                    // termina la conversion
		valor = ADC10MEM;                       // guarda en una variable
		valor = valor>>2;
  		//mostrar_int(valor);
  		while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
  		  UCA0TXBUF=valor;
  		  TA0CCTL0 &= ~CCIFG;

}


