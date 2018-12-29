#include <msp430.h> 
#include "mensaje_Serial.h"
 volatile int z;
 volatile float rpmR;
 volatile float rpmR2;
 volatile int rpm_voltaje;
//--------------------------------------------------------------------------------------------

void serial(){
	P1SEL= BIT1 + BIT2;
	P1SEL2= BIT1+  BIT2;// se configura los p1.1 y P1.2 para servir como comunicacion serial...
	UCA0CTL1 |= UCSSEL_2  +UCSWRST;// SMCLK
	UCA0BR1 = 0;
	UCA0BR0= 104; // VALOR QUE SE CARGA CON UNA FRECUENCIA 16 MHZ Y UNA MODULACION UCBRS0=1...
    UCA0MCTL= UCBRS_1;  // MODULACION...
	UCA0CTL1 &= ~UCSWRST;
	IE2 = UCA0RXIE;
}
void llamar(){
	BCSCTL1= CALBC1_1MHZ;
	DCOCTL= CALDCO_1MHZ;
	P1DIR= 0x01; // p1.0 salida del triac

	TA0CTL= TASSEL_2+MC_2; // smclk-- modo continuo
	TA1CTL= TASSEL_2+MC_1; // smclk-- modo up
	TA1CCTL0= CCIE; // habilito la interrupcion
	TA1CCR0= 2000; // genera una frecuencia de 500 hz osea 2 mS para el muestreo
	}

void interrupcion_pulso(){
	//********* INTERRUPCION POR EL PIN
	P1IE= BIT3; // habilitar la interrupcion por el pin 5...
	P1IES= ~BIT3; // bandera de la tansicion
	P1IFG= ~BIT3; // bandera puesta en 0
	// ********* FIN DE LA CONFIGURACION interupcion por p1.5
}
int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    llamar();
    serial();
    interrupcion_pulso();

    _BIS_SR(GIE);
    while(1){

    }
}
// interrupcion del recepcion del valor de referencia
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
	z=UCA0RXBUF;  // valor recibido de referencia  de processing
	rpmR= (z*80)+100;  // valor maximo 1000

	IFG2&=~UCA0RXIFG;

}
// interrupcion del cruce por cero
#pragma vector= PORT1_VECTOR
__interrupt void Led_ISR (void){
	TA0R= 0; // la cuenta empieza desde yaaa!!
	while(TA0R<=rpmR){}  // esperamos el tiempo necesario para la potencia que queremos.
	P1OUT= BIT0; // activamos el triac
	__delay_cycles(200);
	P1OUT= ~BIT0;  // dejamos en bajo para cuando suceda un cambio de polarizacion el triac se desconecte
	P1IFG^= BIT3; // se cambia la bandera...
}
// Muestreo de la señal




