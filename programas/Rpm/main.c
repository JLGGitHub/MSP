#include <msp430G2553.h>
#include "mensaje_Serial.h"
volatile int vueltas;
int rpm;
void configuraciones(){
	P1DIR= BIT0 + BIT6;  // salidas de los le
 P1DIR= ~BIT4;
	//********* INTERRUPCION POR EL PIN
	P1IE= BIT4; // habilitar la interrupcion por el pin 5...
	P1IES= ~BIT4; // bandera de la tansicion
	P1IFG= ~BIT4; // bandera puesta en 0
	// ********* FIN DE LA CONFIGURACION interupcion por p1.5
	TACTL= TASSEL_2+MC_2; // smclk-- modo continuo

}


void serial(){
	P1SEL= BIT1 + BIT2;
	P1SEL2= BIT1+  BIT2;// se configura los p1.1 y P1.2 para servir como comunicacion serial...
	UCA0CTL1 |= UCSSEL_2  +UCSWRST;// SMCLK
	UCA0BR1 = 0;
	UCA0BR0= 104; // VALOR QUE SE CARGA CON UNA FRECUENCIA 16 MHZ Y UNA MODULACION UCBRS0=1...
    UCA0MCTL= UCBRS_1;  // MODULACION...
	UCA0CTL1 &= ~UCSWRST;

}
void main(void) {


	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	BCSCTL2 = SELM_0;          //MCLK=SMCLK=DCO
	BCSCTL1= CALBC1_1MHZ;
	DCOCTL= CALDCO_1MHZ;//
	configuraciones();
	serial();
	__enable_interrupt();
	
	while(1){
		TAR=0;
		while(TAR<=60000);
		rpm= vueltas* 300;
		mostrar_int(rpm);
		vueltas=0;

	}
}

#pragma vector= PORT1_VECTOR
__interrupt void Led_ISR (void){
	P1OUT^= BIT6;  // aviso que esta entrando a la interrupcion...
	vueltas++;
	P1IFG^= BIT4; // se cambia la bandera...
}
