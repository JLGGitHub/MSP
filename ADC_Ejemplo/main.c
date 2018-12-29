#include <msp430.h>

#define   Num_of_Results   8

volatile unsigned int A0results[Num_of_Results];
volatile unsigned int A1results[Num_of_Results];
volatile unsigned int A2results[Num_of_Results];
volatile unsigned int A3results[Num_of_Results];

int main(void)
{
  WDTCTL = WDTPW+WDTHOLD;                   // Stop watchdog timer
  P6SEL = 0x0F;                             // Enable A/D channel inputs
  ADC12CTL0 = ADC12ON+ADC12MSC+ADC12SHT0_2; // Turn on ADC12, extend sampling time                                        // to avoid overflow of results
  ADC12CTL1 = ADC12SHP+ADC12CONSEQ_3;       // Use sampling timer, repeated sequenc
  ADC12MCTL1 = ADC12INCH_1;                 // ref+=AVcc, channel = A1
  ADC12MCTL3 = ADC12INCH_3+ADC12EOS;        // ref+=AVcc, channel = A3, end seq.
  //ADC12IE = 0x08;                           // Enable ADC12IFG.3
  ADC12CTL0 |= ADC12ENC;                    // Enable conversions
  ADC12CTL0 |= ADC12SC;                     // Start convn - software trigger
  P1DIR=0x01;

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, Enable interrupts
  while(1){
	  while(ADC12BUSY)
	  if( ADC12MEM1>1000) P1OUT^=0x01;
  }

}


#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void)

{

	 if( ADC12MEM1>1000) P1OUT^=0x01;
  }

