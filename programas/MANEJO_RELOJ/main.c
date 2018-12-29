#include <msp430.h> 

void configurar_reloj(){
	BCSCTL2 = SELM_0;
	BCSCTL1= CALBC1_16MHZ;
	DCOCTL= CALDCO_16MHZ;  // 0.125 micro segundos cada ciclo...
}


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	configurar_reloj();
	P1DIR=BIT6;
	while(1){
		P1OUT^= BIT6;
	}

}
