/*
 * mensaje_Serial.h
 * autor: Pepe gomez
 */

#ifndef MENSAJE_SERIAL_H_
#define MENSAJE_SERIAL_H_
#include <string.h>
#include <stdio.h>
char cadena[30];
char x = '-';
int i;

void llamado(){
	BCSCTL1= CALBC1_16MHZ;
	DCOCTL= CALDCO_16MHZ;
	// se configura el reloj a 16 megaHZ
	P1SEL= BIT1 + BIT2;
	P1SEL2= BIT1+  BIT2;
	// se configura los p1.1 y P1.2 para servir como comunicacion serial...
}
void conf_serial(){
	UCA0CTL1= UCSSEL_2; // SMCLK
	UCA0BR0= 62; // VALOR QUE SE CARGA CON UNA FRECUENCIA 1 MHZ Y UNA MODULACION UCBRS0=1...
    UCA0BR1 = 0;
	UCA0MCTL= UCBRS_4; // MODULACION...
    //UCA0MCTL= UCBRF_3 + UCOS16; // MODULACION...
	UCA0CTL1 &= ~UCSWRST;
	IE2 = UCA0RXIE;  // SE ACTIVA LA INTERRUPCION POR RECEPCION.
	__enable_interrupt();
}

#endif /* MENSAJE_SERIAL_H_ */
