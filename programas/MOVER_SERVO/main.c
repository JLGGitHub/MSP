#include <msp430.h> 
#include "mensaje_Serial.h"
char dato;
int m= 700; // grado 10 ....aproximado...
void t_A(){
	TA0CTL= TASSEL_2 + MC_1;
	TA0CCR0= 20000; // para generar una señal de 50 hz
	TA0CCR1= 1500;
	TACCTL1= OUTMOD_7;
	P1DIR= BIT0 + BIT6;  //
	P1SEL= BIT6;  // la salida del pwm...
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    llamado();
    conf_serial();
    t_A();
    while(1){
    }
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
	dato= UCA0RXBUF;
	IFG2&=~UCA0RXIFG;
	P1OUT^=BIT0;

	switch(dato){
				case 'a': {
					m=600;
				} break;
				case 's': {
					m=1500;
				} break;
				case 'd':{
					m=2300;
				} break;
			}
	TACCR1= m;

		}
