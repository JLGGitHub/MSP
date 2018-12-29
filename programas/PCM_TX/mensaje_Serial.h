
/*
 * mensaje_Serial.h
 * autor: Pepe gomez
 */

#ifndef MENSAJE_SERIAL_H_
#define MENSAJE_SERIAL_H_

void llamado(){
	P1SEL= BIT1 + BIT2;
	P1SEL2= BIT1+  BIT2;// se configura los p1.1 y P1.2 para servir como comunicacion serial...
}
void conf_serial(){
	UCA0CTL1= UCSSEL_2 + UCSWRST;// SMCLK
	UCA0BR0= 130; // VALOR QUE SE CARGA CON UNA FRECUENCIA 1 MHZ Y UNA MODULACION UCBRS0=1...
    UCA0BR1 = 6;
    UCA0MCTL= UCBRS_6;  // MODULACION...
	UCA0CTL1 &= ~UCSWRST;
}
#endif /* MENSAJE_SERIAL_H_ */
