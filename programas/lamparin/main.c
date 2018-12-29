#include <msp430G2553.h>

unsigned int adc;

int main(void) {
P1DIR= BIT6;
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    BCSCTL2 = SELM_0;          //MCLK=SMCLK=DCO
    BCSCTL1 = CALBC1_1MHZ;    //configurando
    DCOCTL = CALDCO_1MHZ;     //DCO 16Mhz

    P1SEL = BIT1 + BIT2;    //P1.1 rx P1.2 tx
    P1SEL2 = BIT1 + BIT2;   //P1.1 rx P1.2 tx
	
	ADC10CTL0= SREF_0 + ADC10SHT_2 + ADC10ON;
		ADC10CTL1= INCH_5 + SHS_0 + ADC10DIV_0 + ADC10SSEL_0 + CONSEQ_0;
		ADC10AE0= BIT5;
		ADC10CTL0 |= ENC;

	UCA0CTL1 |= UCSSEL_2 + UCSWRST;  //reloj de UART=SMCLK   se pone en estado reset mientras se configura
	UCA0CTL0 = 0x00;  //modo asincrono UART sin paridad 8-bit un STOP bit
	UCA0BR1 = 0;
	UCA0BR0 = 8;    //PARA 256000 con 16Mhz
	UCA0MCTL = UCBRS_6 + UCBRF_0;
	UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**

	TA0CTL = TASSEL_2 + ID_0 + MC_2;   //fuente SM dividido en 1 continuo
	TA0CCR0 = 1000;                     //estaba con 1000
	TA0CCTL0 = CCIE;                   //interrupcion de cc0

	__enable_interrupt();

	while(1);
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void){		//CCR0 tiene su propio vector de interrupcion
	P1OUT^= BIT6;
	ADC10CTL0 |= ENC + ADC10SC;           // inicia conversion
	while(ADC10CTL0 & ADC10SC);           // espera a que termine SC
	ADC10CTL0 &= ~ENC;                    // termina la conversion
	adc = ADC10MEM;                       // guarda en una variable
	adc = adc>>2;


	while(!(IFG2 & UCA0TXIFG));          //tal vez no sea necesario esperar aca
	UCA0TXBUF = adc;
	TA0CCTL0 &= ~CCIFG;
	TA0CCR0 += 1000;					 //estaba con 1000
}
