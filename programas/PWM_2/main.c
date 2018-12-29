#include  <msp430g2553.h>
volatile int m; // variable que proporcionara el PWM

void llamado(){
	m= 0;
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	DCOCTL= DCO0 + DCO1;
	BCSCTL1= RSEL0; // se crea una señal de reloj de 150 Khz...
	BCSCTL2= SELM1+ SELM0;
	BCSCTL2= SELM0+SELM1;//+DIVS0;
	TACTL= TASSEL_2+MC_1;
	P1SEL= BIT6+BIT2;
	P1DIR= BIT2 + BIT6;
	P1OUT=0x00;
	P1IE= BIT3;
	P1IES= BIT3;
	P1IFG=~BIT3;  // LA BANDERA ARRANCA EN 0
	CCR0= 2000; // para generar una señal de 500 hz
	CCR1= 100; // 5%  de la cuenta.
	TACCTL1 = OUTMOD_7;
	 _BIS_SR(GIE);

}

void main(void){
	llamado();
		while(1){}
}

#pragma vector=PORT1_VECTOR
__interrupt void P1_Interrupt(void) {
	P1IFG=~BIT3;

	m+=300; // cambiar el PWM
	if(m==1800){m=0; }
	CCR1= m;
}
