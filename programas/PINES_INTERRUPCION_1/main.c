#include  <msp430g2553.h>


void configuracion(){
	//Configurando el reloj, a 1 MHZ
	BCSCTL1= CALBC1_1MHZ;
	DCOCTL= CALDCO_1MHZ;
	P1DIR= 0x41; // pin 3 como entrada
	P2DIR= 0x00;
	P1IE= BIT3;  // se habilita la interrupcion
	P1IES= ~BIT3;  // bandera en la transicion
    P1IFG= ~BIT3;  // bandera en 0...
    P2IE= BIT5;  // se habilita la interrupcion
    P2IES= ~BIT5;  // bandera en la transicion
    P2IFG= ~BIT5;  // bandera en 0..*/
    _BIS_SR(GIE);
}

void main(){
	WDTCTL = WDTPW | WDTHOLD;
	configuracion();
	while(1){
	}
}
#pragma vector= PORT1_VECTOR
__interrupt void Led_ISR (void){
	P1OUT^= BIT0;  // se conmuta el led...
	P1IFG^= BIT3; // se cambia la bandera...
}
#pragma vector= PORT2_VECTOR
__interrupt void Led_ISR2 (void){
		P1OUT^=BIT6;
		P2IFG^= BIT5; // se cambia la band

}
