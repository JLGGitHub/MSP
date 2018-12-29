#include <msp430.h> 
#include <stdio.h>
#include <string.h>
#include "QmathLib.h"

char cadena[4];
int m;

void mostrar_serial_text(char *c){
	for(m=0;m<strlen(c); m++){
		while (!(UCA0IFG & UCTXIFG));
		UCA0TXBUF =c[m];
	    	  }
}
void mostrar_serial_int(int x){
	sprintf(cadena,"%d",x);
	for(m=0; m< strlen(cadena); m++){
		while (!(UCA0IFG & UCTXIFG));
		UCA0TXBUF =cadena[m];
	}
}

/*
void conf_ADC(){
	 P6SEL = 0x0F;                             // Enable A/D channel inputs
	  ADC12CTL0 = ADC12ON+ADC12MSC+ADC12SHT0_2; // Turn on ADC12, extend sampling time                                        // to avoid overflow of results
	  ADC12CTL1 = ADC12SHP+ADC12CONSEQ_3;       // Use sampling timer, repeated sequenc
	  ADC12MCTL1 = ADC12INCH_1;                 // ref+=AVcc, channel = A1
	  ADC12MCTL3 = ADC12INCH_3+ADC12EOS;        // ref+=AVcc, channel = A3, end seq.
	  //ADC12IE = 0x08;                           // Enable ADC12IFG.3
	  ADC12CTL0 |= ADC12ENC;                    // Enable conversions
	  ADC12CTL0 |= ADC12SC;                     // Start convn - software trigger

}
*/

void MyReloj_20Mhz(void){
		UCSCTL3 = SELREF_2; 							// Set DCO FLL reference = REFO
		UCSCTL4 |= SELA_2; 								// Set ACLK = REFO
		UCSCTL0 = 0x0000; 								// Set lowest possible DCOx, MOD
			do
			{
			UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
			SFRIFG1 &= ~OFIFG;
			}while (SFRIFG1&OFIFG);

		__bis_SR_register(SCG0);
		UCSCTL1 = DCORSEL_6;
		UCSCTL2 |= 615;                   // Set DCO Multiplier for 12MHz
		__bic_SR_register(SCG0);
		__delay_cycles(700000);							// Tiempo para establcer el reloj.
}
void Iniciar_Uart(void){
	P3SEL |= BIT3+BIT4; 								// P3.3 P3.4 = TXD/RXD
	UCA0CTL1 |= UCSWRST; 								// Reset the state machine-USCI logic held in reset state.
	UCA0CTL1 |= UCSSEL_2; 								// SMCLK como fuente de reloj
	UCA0BR0 = 175; 										// 20MHz/115200 = 175
	UCA0BR1 = 0;
	UCA0MCTL |=  UCBRS_5; 								// Modulation UCBRSx=7, UCBRFx=0
	UCA0CTL1 &= ~UCSWRST; 								// Initialize the state machine
	UCA0IE |= UCRXIE; 									// Enable USCI_A RX interrupt
//============ Configuracion UART =================
}
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD; 		// Stop watchdog timer
	MyReloj_20Mhz();
	Iniciar_Uart();

	_q12 X;
	_q12 P;
	X = _Q12(3.6);
	P = _Q12toF(X);
	//Y = _Q12(7.0);
	//P = _Q12div(X, Y);
	while(1)
	{
		//mostrar_serial_text("3.62\r");
		//mostrar_serial_int(X);
		mostrar_serial_int(P);

	}
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	P1OUT^=0x01;
	UCA0IFG^=UCRXIFG;

}


/*
		ADC12CTL0 |= ADC12SC;                   // Start sampling/conversion
		mostrar_serial_text("\r");
		mostrar_serial_text("Prueba Serial UART y ADC--> ");
		mostrar_serial_text("Canal A0: ");
		mostrar_serial_int(ADC12MEM1>>4);  // Cambiamos la resolucion del ADC a 10 Bits
		__delay_cycles(800);		// Espacio necesario para que funcione el serial correctamente (corregir esto)
		*/
