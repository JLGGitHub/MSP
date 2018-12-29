#include <msp430.h> 
#include <string.h>
#include <stdio.h>
char cadena[50]="\restoy probando como funciona esto...";
char caracter;
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
void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    llamado();
    conf_serial();
	while(1){
		mostrar_cadena(cadena);
	}
}
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
  UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character
}
