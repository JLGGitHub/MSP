/*
 * mensaje_Serial.h
 * autor: Pepe gomez
 */

#ifndef MENSAJE_SERIAL_H_
#define MENSAJE_SERIAL_H_
#include <string.h>
#include <stdio.h>
char cadena[50];
int i;

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
	UCA0BR0= 104; // VALOR QUE SE CARGA CON UNA FRECUENCIA 1 MHZ Y UNA MODULACION UCBRS0=1...
    UCA0BR1 = 0;
	UCA0MCTL= UCBRS0; // MODULACION...
	UCA0CTL1 &= ~UCSWRST;
	IE2 = UCA0RXIE;  // SE ACTIVA LA INTERRUPCION POR RECEPCION.
	_BIS_SR(GIE);  // INTERRUPCIONES GLOBALES
}

void mostrar_cadena(char *c){
	for(i=0; i< sizeof cadena;i++){
		while (!(IFG2&UCA0TXIFG));
		UCA0TXBUF= c[i];
	}
}



#endif /* MENSAJE_SERIAL_H_ */
