#include <msp430.h> 
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

//====================================
volatile unsigned int Contador_interrupcion=0;
volatile unsigned char Captura_rx=NULL;
unsigned char Rx_Bifrost[6];
unsigned char Bits_Control=0;
unsigned char Bits_Direccion=0;
unsigned char Bits_Informacion=0;
unsigned char Auxiliar_Leer_Puerto;
unsigned char Auxiliar_Leer_ADC;
//====================================
#define Deshabilitar_puerto 0x00
#define Escribir_Puerto 0x03
#define Leer_Puerto 0x04
#define Leer_ADC 0x08
#define Habilitar_ADC 0x09
#define Enviar_Cambios 194
#define Longitud 0x23
//====================================
bool Flag_Inicio_Trama= false;
bool Cambio_Entrada_Digital= false;					//<----
bool Cambio_ADC= false;								//<----

char cadena[4];
int m=0;
//====================================
#define flagBifrost 0xf1
#define flagBifrost2 0xf2
//============ Configuracion UART =================
void Iniciar_Uart(void){

	P3SEL |= BIT3+BIT4; 								// P4.4,5 = USCI_A0 TXD/RXD
	UCA0CTL1 |= UCSWRST; 								// Reset the state machine-USCI logic held in reset state.
	UCA0CTL1 |= UCSSEL_2; 								// SMCLK como fuente de reloj
	UCA0BR0 = 72; 										// 8.3MHz/115200 = 72
	UCA0BR1 = 0;
	UCA0MCTL |=  UCBRF_7; 								// Modulation UCBRSx=7, UCBRFx=0
	UCA0CTL1 &= ~UCSWRST; 								// Initialize the state machine
	UCA0IE |= UCRXIE; 									// Enable USCI_A RX interrupt

//============ Configuracion UART =================
}
//============ Configuracion ADC ==================

void Iniciar_ADC(void){
	 P6SEL = 0x0F;                             // Enable A/D channel inputs
 ADC12CTL0 = ADC12ON+ADC12MSC+ADC12SHT0_2; // Turn on ADC12, extend sampling time                                        // to avoid overflow of results
 ADC12CTL1 = ADC12SHP+ADC12CONSEQ_3;       // Use sampling timer, repeated sequenc
 ADC12MCTL1 = ADC12INCH_1;                 // ref+=AVcc, channel = A1
 ADC12MCTL3 = ADC12INCH_3+ADC12EOS;        // ref+=AVcc, channel = A3, end seq.
 //ADC12IE = 0x08;                           // Enable ADC12IFG.3
 ADC12CTL0 |= ADC12ENC;                    // Enable conversions
 ADC12CTL0 |= ADC12SC;                     // Start convn - software trigger
	//ADC12IE = 0x08;                           // Enable ADC12IFG.3
//============ Configuracion ADC ==================
}

void Iniciar_puertos(void){
	P1DIR= 0xFF;   								// Puerto 1 como salidas Digitales
	P2DIR= 0x00;								// Puerto 2 como entradas Digitales
	//P2REN=0xFF;								// Resistencias de pull-up
//============ Configuracion Puertos ==============
}
//============ Configuracion Reloj ==================
void MyReloj(void){
		UCSCTL3 = SELREF_2; 					// Set DCO FLL reference = REFO
		UCSCTL4 |= SELA_2; 						// Set ACLK = REFO
		UCSCTL0 = 0x0000; 						// Set lowest possible DCOx, MOD
			do
			{
			UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
			SFRIFG1 &= ~OFIFG;
			}while (SFRIFG1&OFIFG);

		__bis_SR_register(SCG0);
		UCSCTL1 = DCORSEL_5;
		UCSCTL2 |= 248;							// (248+1)* 32670= 8.3MHz Aproximadamente, Ver Guia de Uart.
		__bic_SR_register(SCG0);
		__delay_cycles(250000);					// Tiempo para establcer el reloj.
}
//============ Configuracion Reloj ==================

//============ Enviar Informacion  ==================
void Enviar_serial_text(char c){
		while (!(UCA0IFG & UCTXIFG));
		UCA0TXBUF =c;
	    	  }

void mostrar_serial_int(int x){
	sprintf(cadena,"%d",x);
	for( m=0; m< strlen(cadena); m++){
		while (!(UCA0IFG & UCTXIFG));
		UCA0TXBUF =cadena[m];
	}
}

//============ Enviar Informacion  ==================


void Metodo_Escribir_Puerto(){
	Auxiliar_Leer_Puerto = 0x03;
	Enviar_serial_text(flagBifrost);
	Enviar_serial_text(Bits_Direccion);
	Enviar_serial_text(Leer_Puerto);
	Enviar_serial_text(P1IN&BIT0);
	Enviar_serial_text(0x01);
	Enviar_serial_text(flagBifrost2);
}
void Metodo_Leer_Puerto(){
	Auxiliar_Leer_Puerto = 0x03;
	Enviar_serial_text(flagBifrost);
	Enviar_serial_text(Bits_Direccion);
	Enviar_serial_text(Leer_Puerto);
	Enviar_serial_text(Auxiliar_Leer_Puerto);
	Enviar_serial_text(0x00);
	Enviar_serial_text(flagBifrost2);

}


void Metodo_Leer_ADC(){

	if(Bits_Direccion==Longitud){
		P1OUT=0x01;
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_Leer_ADC= ADC12MEM0>>4;
		 }else if(Bits_Direccion==Longitud+1){
			 P1OUT=0x00;
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_Leer_ADC= ADC12MEM1>>4;
		 }else if(Bits_Direccion== Longitud+2){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_Leer_ADC= ADC12MEM2>>4;
		 }else if(Bits_Direccion==Longitud+3){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_Leer_ADC= ADC12MEM3>>4;
		 }else if(Bits_Direccion==Longitud+4){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_Leer_ADC= ADC12MEM4>>4;
		 }else if(Bits_Direccion==Longitud+5){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_Leer_ADC= ADC12MEM5>>4;
		 }else if(Bits_Direccion==Longitud+6){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_Leer_ADC= ADC12MEM6>>4;
		 }else if(Bits_Direccion==Longitud+7){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_Leer_ADC= ADC12MEM7>>4;
		 }
	Enviar_serial_text(flagBifrost);
	Enviar_serial_text(Bits_Direccion);
	Enviar_serial_text(Leer_ADC);
	Enviar_serial_text(Auxiliar_Leer_ADC);
	Enviar_serial_text(0x00);
	Enviar_serial_text(flagBifrost2);

	//ADC12CTL0 |= ADC12SC;
	//mostrar_serial_int(ADC12MEM1>>4);
	//if((ADC12MEM1>>4)>100)
		//P1OUT=0x01;
	//else
		//P1OUT=0x00;
		//Enviar_serial_text((ADC12MEM1>>4));
	}

void Metodo_Habilitar_ADC(){
	 if(Bits_Direccion==Longitud){
		 ADC12MCTL1 = ADC12INCH_0;   				// ref+=AVcc, channel = A0

	 }else if(Bits_Direccion==Longitud+1){
		 ADC12MCTL1 = ADC12INCH_1;                 // ref+=AVcc, channel = A1

	 }else if(Bits_Direccion==Longitud+2){
		 ADC12MCTL1 = ADC12INCH_2;                 // ref+=AVcc, channel = A2

	 }else if(Bits_Direccion==Longitud+3){
		 ADC12MCTL1 = ADC12INCH_3;                 // ref+=AVcc, channel = A3

	 }else if(Bits_Direccion==Longitud+4){
		 ADC12MCTL1 = ADC12INCH_4;                 // ref+=AVcc, channel = A4

	 }else if(Bits_Direccion==Longitud+5){
		 ADC12MCTL1 = ADC12INCH_5;                 // ref+=AVcc, channel = A5

	 }else if(Bits_Direccion==Longitud+6){
		 ADC12MCTL1 = ADC12INCH_6;                 // ref+=AVcc, channel = A6

	 }else if(Bits_Direccion==Longitud+7){
		 ADC12MCTL1 = ADC12INCH_7;                 // ref+=AVcc, channel = A7
	 }
}
void Metodo_cambios(){

}


////////////////////////////////////////////////////////
void Recepcion_Bifrost(){
	if(Captura_rx==flagBifrost) 	Flag_Inicio_Trama= true;
	if(Flag_Inicio_Trama==true)		{Rx_Bifrost[Contador_interrupcion]= Captura_rx;}
	if(Contador_interrupcion==6){
		Flag_Inicio_Trama=false;
		Contador_interrupcion=0;
		Bits_Direccion  = 	Rx_Bifrost[2];
		Bits_Control    =	Rx_Bifrost[3];
		Bits_Informacion= 	Rx_Bifrost[4];

		// ---> Aqui va la funcion de CRC <---
		// preguntar si el CRC calculado es igual al recibido en la trama, y preguntar si la bandera de final corresponde al dato que entra
		//============================================
		if(Rx_Bifrost[6]==flagBifrost2){
			if(Bits_Control== Escribir_Puerto){
				Metodo_Escribir_Puerto();
			}else if(Bits_Control== Leer_Puerto){
				//Enviar_serial_text(0xff);
				Metodo_Leer_Puerto();
			}else if(Bits_Control== Leer_ADC){
				Metodo_Leer_ADC();
			}else if(Bits_Control==Habilitar_ADC){
				Metodo_Habilitar_ADC();
			}else if(Bits_Control==Enviar_Cambios){
				Metodo_cambios();
			}
		}
		//============================================
	}
}
/////////////////////////////////////////////////////////
void main(void) {
    WDTCTL = WDTPW | WDTHOLD;					// Stop watchdog timer
    MyReloj();
    Iniciar_puertos();
    Iniciar_ADC();
    Iniciar_Uart();
    __bis_SR_register(GIE); 					// Enable interrupts Globals
    while(1){
    }
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){
	//P1OUT^=0x01;
	Contador_interrupcion+=1;
	Captura_rx=UCA0RXBUF;
	Recepcion_Bifrost();

}
