#include <msp430.h>
volatile unsigned char A0results = 0xFF;
/*
* main.c
*/
void UART_write(volatile unsigned char txdata_u16);
void config_UART_wired(void);
void config_CLOCK(void);
int main(void)
{
	P1DIR=0x01;
	P1OUT=0x00;


WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
config_CLOCK();
config_UART_wired();
while(1)
{
	P1OUT^=0x01;
	__delay_cycles(1000000);
}
}

void config_UART_wired(void)
{
/****************************************** UART Config wired ***************************************************************/

P3SEL |= BIT3+BIT4; // P4.4,5 = USCI_A1 TXD/RXD
UCA0CTL1 |= UCSWRST; // Reset the state machine-USCI logic held in reset state.
UCA0CTL1 |= UCSSEL_2; // SMCLK as source clk
UCA0BR0 = 69; // 1MHz/115200 = 72
UCA0BR1 = 3;
UCA0MCTL |=  UCBRF_7; // Modulation UCBRSx=7, UCBRFx=0
UCA0CTL1 &= ~UCSWRST; // Initialize the state machine
UCA0IE |= UCRXIE; // Enable USCI_A RX interrupt

// __delay_cycles(50000); //delay to stabilise clock
__bis_SR_register(GIE); // Enable interrupts
/****************************************** UART Config wired ***************************************************************/

}

void UART_write(volatile unsigned char txdata_u16)
{
int i;
for(i=0;i<100;i++);
while (!(UCA0IFG & UCTXIFG)); // Is the USCI_A1 TX buffer ready?
UCA0TXBUF = txdata_u16; // TX -> LSB

}
void config_CLOCK(void)
{
volatile unsigned int i;

P1DIR |= BIT1; // P1.1 output
//P1DIR |= BIT0; // ACLK set out to pins
//P1SEL |= BIT0;
//P2DIR |= BIT2; // SMCLK set out to pins
//P2SEL |= BIT2;
//P7DIR |= BIT7; // MCLK set out to pins
//P7SEL |= BIT7;
UCSCTL3 = SELREF_2; // Set DCO FLL reference = REFO
UCSCTL4 |= SELA_2; // Set ACLK = REFO
UCSCTL0 = 0x0000; // Set lowest possible DCOx, MODx

// Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
do
{
UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
// Clear XT2,XT1,DCO fault flags
SFRIFG1 &= ~OFIFG; // Clear fault flags
}while (SFRIFG1&OFIFG); // Test oscillator fault flag

__bis_SR_register(SCG0); // Disable the FLL control loop
UCSCTL1 = DCORSEL_5; // Select DCO range 16MHz operation
UCSCTL2 |= 249; // Set DCO Multiplier for 8MHz
// (N + 1) * FLLRef = Fdco
// (249 + 1) * 32768 = 8MHz
__bic_SR_register(SCG0); // Enable the FLL control loop

// Worst-case settling time for the DCO when the DCO range bits have been
// changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
// UG for optimization.
// 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
__delay_cycles(250000);

}

// This is the RX UART ISR and is entered when the RX buffer is full
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	P1OUT^=0x01;
UART_write('f');
UCA0IFG^=UCRXIFG;
}
