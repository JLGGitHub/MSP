#include <msp430.h> 
int valor;
int valor_adc;
void llamado(){
	BCSCTL1= CALBC1_1MHZ;
	DCOCTL= CALDCO_1MHZ;
	// se configura el reloj a 1 megaHZ
	P1SEL= BIT1 + BIT2;
	P1SEL2= BIT1+  BIT2;
	// se configura los p1.1 y P1.2 para servir como comunicacion serial...
}
void conf_serial(){
	UCA0CTL1= UCSSEL_2; // SMCLK
	UCA0BR0= 104	; // VALOR QUE SE CARGA CON UNA FRECUENCIA 16 MHZ Y UNA MODULACION UCBRS0=1...
    UCA0BR1 = 0;
	UCA0MCTL= UCBRS0; // MODULACION...
   // UCA0MCTL= UCBRS2+ UCBRS1+ UCBRS0;
	UCA0CTL1 &= ~UCSWRST;
	IE2 = UCA0RXIE;  // SE ACTIVA LA INTERRUPCION POR RECEPCION.
	_BIS_SR(GIE);  // INTERRUPCIONES GLOBALES
}
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

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    muestreo();
	llamado();
	adc();
	conf_serial();
	P1DIR |= BIT0;
	
	while(1){}
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void)
{	P1OUT^= BIT6;
	TA0CCR0+=5000;
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

