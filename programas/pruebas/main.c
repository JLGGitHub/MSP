#include <msp430.h>
#include <stdio.h>
#include <string.h>

int distancia;
int triger;
int tiempo;
int con;
int i;
char CADENA[13];
char cadena[20];
void llamado(){
	        	DCOCTL = CALDCO_1MHZ; // Set DCO to 1MHz
			BCSCTL1 = CALBC1_1MHZ; // Set DCO to 1MHz
			BCSCTL2= SELM0+SELM1;//+DIVS0;
			TACTL= TASSEL_2+MC_2;
			P2DIR= 0x04;  // se define p2.2 como salida y p2.5 como entrada...
			_BIS_SR(GIE);
}
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
void mostrar_serial(char *c){
			for(i=0;i<13;i++){
	    	  while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	    	     UCA0TXBUF = c[i];
	    	  }
}
void mostrar_serial_int(int x){
	       sprintf(cadena,"\rDistancia: %d cm   ",x);
			for(i=0; i< 20; i++){
	    	  while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	    		UCA0TXBUF =cadena[i];
			}
	}

void ultra_sonido(){
			P2OUT=~BIT2;
			__delay_cycles(7);
			P2OUT=BIT2;
			__delay_cycles(20);
			P2OUT=~BIT2;
			while(!(BIT5 & P2IN)){}
			TAR=0;
			while(BIT5 & P2IN){}
			tiempo= TAR;
			distancia= (0.034* tiempo)/2;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    llamado();
    serial();

while(1){
		ultra_sonido();
		mostrar_serial_int(distancia);
	}
}
