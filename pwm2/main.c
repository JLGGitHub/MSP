#include "msp430g2553.h"
#include <stdio.h>
#include <string.h>



volatile int segundo;
int i;
char CADENA[13];
char cadena[5];
__interrupt void Timer_A(void);
void serial(){

	            P1SEL = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
			    P1SEL2 = BIT1 + BIT2 ; // P1.1 = RXD, P1.2=TXD
			    UCA0CTL1 |= UCSSEL_2; // Use SMCLK
			    UCA0BR0 = 104; // Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13)
			    UCA0BR1 = 0; // Set baud rate to 9600 with 1MHz clock
			    UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1
			    UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
			    //IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
}
void llamado(){
	        DCOCTL = CALDCO_1MHZ; // Set DCO to 1MHz
			BCSCTL1 = CALBC1_1MHZ; // Set DCO to 1MHz
			BCSCTL2= SELM0+SELM1;//+DIVS0;
			P1DIR= BIT0; // LED ROJO COMO SALIDA...
			WDTCTL= WDTPW+WDTHOLD;
			_BIS_SR(GIE);
}

void timer(){
	        CCTL0=CCIE;  // HABILITA LA INTERRUPCION
		    CCR0= 62000;
		    TACTL= TASSEL_2+MC_3+ID_3; // DIVICION DE RELOJ EN 8.
}

void mostrar_serial(char *c){
			for(i=0;i<13;i++){
	    	  while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	    	     UCA0TXBUF = c[i];
	    	  }
}
void mostrar_serial_int(int x){
	       sprintf(cadena,"%d   ",x);
			for(i=0; i< 5; i++){
	    	  while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	    		UCA0TXBUF =cadena[i];
			}
	}

int main(void) {
	    llamado();
	    timer();
	    serial();

    while(1){
    	 //mostrar_serial("\rsegundos: ");
    	 mostrar_serial_int(segundo);

    }
}
#pragma vector= TIMER0_A0_VECTOR
__interrupt void Timer_A(){
  P1OUT^=BIT0; // OPERACION XNOR CADA QUE ENTRA A LA INTERRUPCION.
 segundo+=1;
}


