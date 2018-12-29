#include <msp430.h> 
#include "mensaje_Serial.h"
char cadena[50]="\rastoy probando como funciona esto...";

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	llamado();
	conf_serial();
	while(1){
		mostrar_cadena(cadena);
	}
}
