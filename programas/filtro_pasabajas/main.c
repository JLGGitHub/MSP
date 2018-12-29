#include <msp430.h> 
// filtro pasabajas...........
#define m 17
int n=17
float x[];
float y[];
float h[m];
 float yy;
int i=0;


float h[m]={
		0.037833,//h0
		0.040505,
		0.042904,
		0.044997,
		0.046753,//h4
		0.048147,
		0.049157,
		0.049769,
		0.499774,//h8
		0.049769,
		0.049157,
		0.048147,
		0.046753,//h12
		0.044997,
		0.042904,
		0.040505,
		0.037833//h16
	};

void adc(){
	ADC10CTL0= SREF_0 + ADC10SHT_2 + ADC10ON;
	ADC10CTL1= INCH_5 + SHS_0 + ADC10DIV_0 + ADC10SSEL_0 + CONSEQ_0;
	ADC10AE0= BIT5;
	ADC10CTL0 |= ENC;
}

void muestreo(){
	TA0CTL = TASSEL_2  + ID_0 + MC_2;
	//TA0CCR0 = 1000;                     //estaba con 1000
	TA0CCTL0= CCIE;   // interrupcion habilitada
	P1DIR |= BIT6;
}

int main(void) {
	BCSCTL2 = SELM_0;          //MCLK=SMCLK=DCO
	BCSCTL1= CALBC1_16MHZ;
	DCOCTL= CALDCO_16MHZ;// se configura el reloj a 16 megaHZ
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    adc();
	muestreo();
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void)
{
	P1OUT^= BIT6;
	TA0CCR0+=1000;
	for(i=n-1;i!=0;i--){ x[i]=x[i-1];}
		ADC10CTL0 |=  ADC10SC;
		while(ADC10CTL1 & ADC10BUSY); //esperamos que este lista la conversion
		x[0] = ADC10MEM>>1;                       // guarda en una variable
	    y=0;
	    for(i=0;i<n;i--){ y+=h[i]*x[i];}

	    yy=y<<1;// salida
		//while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
  		 //UCA0TXBUF=valor;
  		  //TA0CCTL0 &= ~CCIFG;

}
