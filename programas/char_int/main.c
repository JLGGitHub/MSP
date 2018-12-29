#include "msp430g2553.h"
#include <stdio.h>
#include <string.h>
 char cadena[3];
 int entero=26;

int i;

void llamado(){
	        DCOCTL = CALDCO_1MHZ; // Set DCO to 1MHz
			BCSCTL1 = CALBC1_1MHZ; // Set DCO to 1MHz
			BCSCTL2= SELM0+SELM1;//+DIVS0;
			//P1DIR= BIT0; // LED ROJO COMO SALIDA...
			WDTCTL= WDTPW+WDTHOLD;
			_BIS_SR(GIE);
            P1SEL = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
		    P1SEL2 = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
		    UCA0CTL1 |= UCSSEL_2; // Use SMCLK
		    UCA0BR0 = 104; // Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13)
		    UCA0BR1 = 0; // Set baud rate to 9600 with 1MHz clock
		    UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1
		    UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
		    //IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
}

void mostrar_serial_int(int x){
	       sprintf(cadena,"\r%d",x);
			for(i=0; i< 3; i++){
	    	  while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	    		UCA0TXBUF =cadena[i];
			}
	}

int main(void) {
   llamado();
   while(1){
	   mostrar_serial_int(entero);
   }

	
}
