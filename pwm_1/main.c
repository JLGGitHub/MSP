#include <msp430g2553.h>
#include <stdio.h>
#include <string.h>

volatile int segundo; // se declara volatile por que es una variable que cambia en una interrupcion
int i; // contador del for..
char dato;
char CADENA[20]; // para los mensajes
char cadena[3]; // para los numeros int8 max 256

__interrupt void timer_A(void);

void serial(){
	P1SEL= BIT1+BIT2;
	P1SEL2=BIT1+BIT2;
	UCA0CTL1= UCSSEL_2; // uso de SMCLK
	UCA0BR0=8; // 1MHZ/104= 9600;
	UCA0BR1= 0;
	UCA0MCTL = UCBRS_6; // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;
	IE2= UCA0RXIE; // ACTIVA LA INTERRUPCION POR RECEPCION
}

void timer(){
	CCTL0= CCIE; // activa la interrupcion de comparacion
	CCR0= 62000; // aproximadamente medio segundo con un reloj de 1 mhz
	TACTL= TASSEL_2+MC_3+ID_3; //divide en 8 el reloj, opcion UP/dawn, ------
}

void llamado(){
	DCOCTL= CALDCO_1MHZ;
	BCSCTL1= CALBC1_1MHZ;
	BCSCTL2= SELM0+SELM1;
	P1DIR= BIT0;
	_BIS_SR(GIE);
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
   WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    llamado();
    serial();
    timer();
		while(1){
			mostrar_serial("\rSegundos: ");
			mostrar_serial_int(segundo);
		}

}
#pragma vector= TIMER0_A0_VECTOR
__interrupt void Timer_A(){
  P1OUT^=BIT0; // OPERACION XNOR CADA QUE ENTRA A LA INTERRUPCION.
 segundo+=1;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {

		dato= UCA0RXBUF; // lo que se reciba de la interrupcion

			switch(dato){
			case 'a': {
				CCTL0=~CCIE;  // se deshabilita la interrupcion de comparacion
				dato='s';
			} break;
			case 'b': {
				CCTL0=CCIE;
				dato='s';
			} break;
			case 'c':{
				segundo=0;
				dato='s';
			} break;
		}
			IFG2&=~UCA0RXIFG;  // borrado de la bandera...
	}
