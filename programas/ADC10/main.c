#include <msp430.h> 
#include "mensaje_Serial.h"
int valor;
int valor_adc;
void adc(){
	ADC10CTL0= SREF_0 + ADC10SHT_2 + ADC10ON;
	ADC10CTL1= INCH_5 + SHS_0 + ADC10DIV_0 + ADC10SSEL_0 + CONSEQ_0;
	ADC10AE0= BIT5;
	ADC10CTL0 |= ENC;
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	llamado();
	adc();
	conf_serial();
	P1DIR |= BIT0;
	
	while(1){

		ADC10CTL0 |=  ADC10SC;
		while(ADC10CTL1 & ADC10BUSY); //esperamos que este lista la conversion
		if (ADC10MEM<512){
			P1OUT= BIT0;}
		else{
			P1OUT &= ~BIT0;
		}
		valor= ADC10MEM>>2;

		 while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
		 UCA0TXBUF=(valor);


	}
}
