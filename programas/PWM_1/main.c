
#include <msp430g2553.h>
volatile int m;

void config(){
	DCOCTL= DCO0 + DCO1;
	BCSCTL1= RSEL0; // se crea una señal de reloj de 150 Khz...
	BCSCTL2= SELM1+ SELM0;
	P1DIR=  BIT0;
	TA0CTL= TASSEL_2 + MC_1 + ID_3;  // fuente de señal smclk y modo UP. dividida en 8...
	TA0CCTL0= CCIE; // activamos la interrupcion por CCRO...
	CCR0= 18750; // valor qeu se carga para una interrupcion cada segundo...
//########################################################################################
	// configurndo pwm por el puerto 2...
	TA1CTL=TASSEL_2 + MC_1;
	P2DIR=BIT1;
	P2SEL=BIT1; // salida de pwm;
	TA1CCR0 =1000;  //ta para p2.1
	TA1CCR1 = 100;
	TA1CCTL1= OUTMOD_7;
//#########################################################################################
	TACCTL1 = OUTMOD_7;
	_BIS_SR(GIE);
}
void main(void) {


	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    config();
    while(1){

    }
}

#pragma vector= TIMER0_A0_VECTOR
__interrupt void Timer_A (void){
	P1OUT ^= BIT0;
	m= m+100;
	if(m>950){
		m= 10;
	}
	TA1CCR1=m;
}
