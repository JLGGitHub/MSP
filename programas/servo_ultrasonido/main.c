#include <msp430.h> 
#include "mensaje_Serial.h"

int distancia;
int tiempo;
int m;
void t_A(){
	TA0CTL= TASSEL_2 + MC_1;
	TA0CCR0= 20000; // para generar una señal de 50 hz
	TA0CCR1= 1500;
	TACCTL1= OUTMOD_7;
	P1DIR= BIT0 + BIT6;  //
	P1SEL= BIT6;  // la salida del pwm...
}

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
    t_A();
    P2DIR= 0x04;
    while(1){
    	ultra_sonido();

    	mostrar_cadena("\rLa distancia del objeto es de: -");
    	mostrar_int(distancia);
    }

	
}
