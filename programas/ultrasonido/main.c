#include <msp430.h> 
#include "mensaje_Serial.h"

int distancia;
int triger;
int tiempo;
int con;

void ultra_sonido(){
			P2OUT=~BIT2;
			__delay_cycles(5);
			P2OUT=BIT2;
			__delay_cycles(11);
			P2OUT=~BIT2;
			while(!(BIT5 & P2IN)){}
			TAR=0;
			while(BIT5 & P2IN){}
			tiempo= TAR;
			distancia= (0.034* tiempo)/2;
}


void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    llamado();
    conf_serial();
    P2DIR= 0x04;
    TACTL= TASSEL_2+MC_2;
    while(1){
    	ultra_sonido();
    	mostrar_cadena("\rLa distancia del objeto es de: -");
    	mostrar_int(distancia);
    }

	
}
