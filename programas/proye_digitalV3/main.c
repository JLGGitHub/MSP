#include  <msp430g2553.h>
volatile int z;
volatile int planta;
volatile int contador_pulsos=1;
 void serial(){
 	P1SEL= BIT1 + BIT2;
 	P1SEL2= BIT1+  BIT2;// se configura los p1.1 y P1.2 para servir como comunicacion serial...
 	UCA0CTL1 |= UCSSEL_2  +UCSWRST;// SMCLK
 	UCA0BR1 = 0;
 	UCA0BR0= 104; // VALOR QUE SE CARGA CON UNA FRECUENCIA 16 MHZ Y UNA MODULACION UCBRS0=1...
     UCA0MCTL= UCBRS_1;  // MODULACION...
 	UCA0CTL1 &= ~UCSWRST;
 	IE2 = UCA0RXIE;
 }

 void configuracion(){
 	//Configurando el reloj, a 1 MHZ
 	BCSCTL1= CALBC1_1MHZ;
 	DCOCTL= CALDCO_1MHZ;
 	TA0CTL= TASSEL_2 + MC_1; // smclk-- modo up
 	TA1CTL= TASSEL_2 + MC_1; // smclk-- modo ip
 	TA1CCTL0= CCIE; // habilito la interrupcion
 	TA0CCTL0= CCIE; // habilito la interrupcion
 	TA1CCR0= 30000; // 100 hz, 10 mS
 	P1DIR= 0x41; // pin 3 como entrada
 	P2DIR= 0x00;
 	P1IE= BIT3;  // se habilita la interrupcion
 	P1IES= ~BIT3;  // bandera en la transicion
    P1IFG= ~BIT3;  // bandera en 0..
    P2IE= BIT5;  // se habilita la interrupcion
    P2IES= ~BIT5;  // bandera en la transicion
    P2IFG= ~BIT5;  // bandera en 0..*/
    _BIS_SR(GIE);
 }
 void main(){

	WDTCTL = WDTPW | WDTHOLD;
 	configuracion();
 	serial();
 	while(1){
 		}
 }
 #pragma vector= PORT1_VECTOR
 __interrupt void Led_ISR (void){
	 	TA0CCR0= planta;
	 	TA0R=0;
	 	TA0CTL= TASSEL_2 + MC_1; // smclk-- modo ip
	  	P1OUT&= ~BIT0;		// triac inactivo
		P1IFG&= ~BIT3; // se cambia la bandera...
 }

#pragma vector=TIMER0_A0_VECTOR    // Timer1 A0 interrupt service routine
__interrupt void Timer0_A0 (void) {
	P1OUT |= BIT0;		// triac activo
	TA0CCTL0 &= ~CCIFG;
	}
 #pragma vector= PORT2_VECTOR
 __interrupt void Led_ISR2 (void){
	 	contador_pulsos++;
 		P2IFG= ~BIT5; // se cambia la band
 }
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
	z=UCA0RXBUF;  // valor recibido de referencia  de processing
	planta= ((z)*80);
	IFG2&=~UCA0RXIFG;
}
#pragma vector=TIMER1_A0_VECTOR    // Timer1 A0 30 MS PARA MUESTREO...
__interrupt void Timer1_A0 (void) {
	P1OUT^=BIT6;
	while (!(IFG2&UCA0TXIFG));
	UCA0TXBUF=contador_pulsos; // se envia la cantida de pulsos (ese valor se debe pasar a rpm en processing)...
	contador_pulsos=0;		// reinicimos la cuenta de pulsos...
	TA1CCTL0 &= ~CCIFG;
	}
