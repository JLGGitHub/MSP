#include <msp430.h> 
#include "mensaje_Serial.h"
/*
 * main.c
 */

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	llamado();
	conf_serial();
	while(1){
		mostrar_cadena("\rla temperatura en el cuarto es de : -");
		mostrar_int(40);
		mostrar_cadena(" grados celsius.-");
	}
	
}
