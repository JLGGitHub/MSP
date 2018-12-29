#include <msp430.h> 
#include <stdio.h>
#include <string.h>

char cadena[4];
int m;

void mostrar_serial_text(char *c){
	for(m=0;m<strlen(c); m++){
		while (!(UCA1IFG & UCTXIFG));
		UCA1TXBUF =c[m];
	    	  }
}
void mostrar_serial_int(int x){
	sprintf(cadena,"%d",x);
	for(m=0; m< strlen(cadena); m++){
		while (!(UCA1IFG & UCTXIFG));
		UCA1TXBUF =cadena[m];
	}
}
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
void config_CLOCK(void)
{
volatile unsigned int i;

P1DIR |= BIT0;


	UCSCTL3 = SELREF_2; // Set DCO FLL reference = REFO
	UCSCTL4 |= SELA_2; // Set ACLK = REFO
	UCSCTL0 = 0x0000; // Set lowest possible DCOx, MOD
		do
		{
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
		SFRIFG1 &= ~OFIFG;
		}while (SFRIFG1&OFIFG);

	__bis_SR_register(SCG0);
	UCSCTL1 = DCORSEL_5;
	UCSCTL2 |= 248;
	__bic_SR_register(SCG0);
	__delay_cycles(250000);

}
void config_UART_wired(void)
{
	P4SEL |= BIT5+BIT4; // P4.4,5 = USCI_A1 TXD/RXD
		UCA1CTL1 |= UCSWRST; // Reset the state machine-USCI logic held in reset state.
		UCA1CTL1 |= UCSSEL_2; // SMCLK as source clk
		UCA1BR0 = 72; // 1MHz/115200 = 72
		UCA1BR1 = 0;
		UCA1MCTL |=  UCBRF_7; // Modulation UCBRSx=7, UCBRFx=0
		UCA1CTL1 &= ~UCSWRST; // Initialize the state machine
		UCA1IE |= UCRXIE; // Enable USCI_A RX interrupt
		__bis_SR_register(GIE); // Enable interrupts
}

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD; 		// Stop watchdog timer
	config_CLOCK();
	config_UART_wired();
	conf_ADC();
	while(1)
	{

		ADC12CTL0 |= ADC12SC;                   // Start sampling/conversion
		mostrar_serial_text("\r");
		mostrar_serial_text("Prueba Serial UART y ADC--> ");
		mostrar_serial_text("Canal A0: ");
		mostrar_serial_int(ADC12MEM1>>4);  // Cambiamos la resolucion del ADC a 10 Bits
		if((ADC12MEM1>>4)>100)
			P1OUT=0x01;
		else
			P1OUT=0x00;
		__delay_cycles(80000);		// Espacio necesario para que funcione el serial correctamente (corregir esto)
	}
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
	P1OUT^=0x01;
	UCA1IFG^=UCRXIFG;

}
