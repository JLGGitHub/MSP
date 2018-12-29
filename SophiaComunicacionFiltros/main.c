#include <msp430.h> 
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include  "CrcLib.h"


//====================================
volatile unsigned int Contador_interrupcion=0;
volatile unsigned char Captura_rx=NULL;
unsigned short inf[10];
unsigned char Rx_Bifrost[6];
unsigned char Bits_Control=0;
unsigned char Bits_Direccion=0;
unsigned char Bits_Informacion=0;
unsigned char Bits_Crc=0;
unsigned char Bits_Crc_Calculado=0;
unsigned char Auxiliar_Leer_Puerto;
unsigned char Auxiliar_ADC;
unsigned short  Informacion_Control, Informacion_P_ADC, Informacion_P_Digital;

//====================================
#define Escribir_Puerto 0x03							// Escribe en el Puerto
#define Leer_Puerto 0x05								// Lee el valor del Puerto
#define Leer_ADC 0x09									// Lee el valor del ADC
#define Configuracion_Inicial 0x0C						// Activa las entradas
#define Desactivado 0x00								// Valor NULL
#define Enviar_Cambios 0x02								// Envia Todos los Datos
#define Longitud 0x22									// Direccion de tarjeta 1
//#define Longitud 0x2C									// Direccion de tarjeta 2
//#define Longitud 0x36									// Direccion de tarjeta 3
//#define Longitud 0x40									// Direccion de tarjeta 4
//====================================
bool Flag_Inicio_Trama= false;
bool Bandera_int_Rx=false;								// Bandera para capturar Trama
bool Bandera_int_Timer=false;							// Bandera para Actualizar Datos
bool Bit_Habilitar[10];									// Bandera para Habilitar Puertos
char cadena[4];											// Envio de Datos
int m=0;
//====================================
#define Flag_Inicio 	0xc8							// Tarjeta 1.
//#define Flag_Inicio 	0xc9							// Tarjeta 2.
//#define Flag_Inicio 	0xCA							// Tarjeta 3.
//#define Flag_Inicio 	0xCB							// Tarjeta 4.
#define Flag_Fin 		0xFD							// Fin de Trama.
//============ Configuracion UART =================

//============ Filtro digital de 10Hz===============
#define b0 1   //Valores para filtro pasabajas de 10Hz 50 muestras por segundo
#define b1 -1.1430
#define b2 0.4128
#define a0 0.0675
#define a1 0.1349
#define a2 0.0675

struct Valores_filtros {
	float x0;
	float y0;
	float x1;
	float y1;
	float x2;
	float y2;
};

struct Valores_filtros VFiltros[8];
//============ Filtro digital de 10Hz===============

void Iniciar_Uart(void){
	P3SEL |= BIT3+BIT4; 								// P3.3 P3.4 = TXD/RXD
	UCA0CTL1 |= UCSWRST; 								// Reset the state machine-USCI logic held in reset state.
	UCA0CTL1 |= UCSSEL_2; 								// SMCLK como fuente de reloj
	UCA0BR0 = 106; 										// 8.3MHz/115200 = 72
	UCA0BR1 = 0;
	UCA0MCTL |=  UCBRS_1; 								// Modulation UCBRSx=7, UCBRFx=0
	UCA0CTL1 &= ~UCSWRST; 								// Initialize the state machine
	UCA0IE |= UCRXIE; 									// Enable USCI_A RX interrupt
//============ Configuracion UART =================
}
//============ Configuracion ADC ==================

void Iniciar_ADC(void){
	P6SEL = 0x0F;                             			// Enable A/D channel inputs
	ADC12CTL0 = ADC12ON+ADC12MSC+ADC12SHT0_2; 			// Turn on ADC12, extend sampling time                                        // to avoid overflow of results
	ADC12CTL1 = ADC12SHP+ADC12CONSEQ_3;       			// Use sampling timer, repeated sequenc
	ADC12MCTL0 = ADC12INCH_0;                 			// ref+=AVcc, channel = A0
	ADC12MCTL1 = ADC12INCH_1;                 			// ref+=AVcc, channel = A1
	ADC12MCTL2 = ADC12INCH_2;                 			// ref+=AVcc, channel = A2
	ADC12MCTL3 = ADC12INCH_3;                 			// ref+=AVcc, channel = A3
	ADC12MCTL4 = ADC12INCH_4;                 			// ref+=AVcc, channel = A4
	ADC12MCTL5 = ADC12INCH_5;                 			// ref+=AVcc, channel = A5
	ADC12MCTL6 = ADC12INCH_6;                 			// ref+=AVcc, channel = A6
	ADC12MCTL7 = ADC12INCH_7;                 			// ref+=AVcc, channel = A7
	ADC12MCTL7 = ADC12EOS;        			  			// ref+=AVcc, channel = A7, end seq.
	ADC12CTL0 |= ADC12ENC;                    			// Enable conversions
	ADC12CTL0 |= ADC12SC;                     			// Start convn - software trigger
//============ Configuracion ADC ==================
}

//============ Configuracion Timer ================

void Iniciar_Timer(void){
	TA0CCTL0 = CCIE;                          			// CCR0 interrupt enabled
	//TA0CCR0 = 20750;									// se carga para 20 Ms en caso de que el reloj este en 8.3 Mhz
	TA0CCR0 = 30720;									// se carga para 20 Ms en caso de que el reloj este en 12.3 Mhz
	TA0CTL = TASSEL_2+MC_1+ID_3;         				// SMCLK, Modo UP, Divi 8.

//============ Configuracion Timer ================
}

void Iniciar_puertos(void){
	P1DIR= 0xFF;   										// Puerto 1 como salidas Digitales
	P2DIR= 0x00;										// Puerto 2 como entradas Digitales
	P2REN|= 0x1F;
//============ Configuracion Puertos ==============
}
//============ Configuracion Reloj ==================
void MyReloj_8Mhz(void){
		UCSCTL3 = SELREF_2; 							// Set DCO FLL reference = REFO
		UCSCTL4 |= SELA_2; 								// Set ACLK = REFO
		UCSCTL0 = 0x0000; 								// Set lowest possible DCOx, MOD
			do
			{
			UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
			SFRIFG1 &= ~OFIFG;
			}while (SFRIFG1&OFIFG);

		__bis_SR_register(SCG0);
		UCSCTL1 = DCORSEL_5;
		UCSCTL2 |= 248;									// (248+1)* 32768 = 8.3MHz Aproximadamente, Ver Guia de Uart.
		__bic_SR_register(SCG0);
		__delay_cycles(700000);							// Tiempo para establcer el reloj.
}

void MyReloj_12Mhz(void){
		UCSCTL3 = SELREF_2; 							// Set DCO FLL reference = REFO
		UCSCTL4 |= SELA_2; 								// Set ACLK = REFO
		UCSCTL0 = 0x0000; 								// Set lowest possible DCOx, MOD
			do
			{
			UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
			SFRIFG1 &= ~OFIFG;
			}while (SFRIFG1&OFIFG);

		__bis_SR_register(SCG0);
		UCSCTL1 = DCORSEL_5;
		UCSCTL2 = FLLD_1 + 374;                   // Set DCO Multiplier for 12MHz
		__bic_SR_register(SCG0);
		__delay_cycles(375000);							// Tiempo para establcer el reloj.
}

//============ Configuracion Reloj ==================

//============ Enviar Informacion  ==================
void Enviar_serial_text(char c){
		while (!(UCA0IFG & UCTXIFG));
		UCA0TXBUF =c;
	    	  }
//============ Enviar Informacion  ==================
void Metodo_Escribir_Puerto(){
	P1OUT= Bits_Informacion;
}
void Metodo_Leer_Puerto(){

	Auxiliar_Leer_Puerto= 			P2IN & 0x80;
	Informacion_Control= 			Auxiliar_Leer_Puerto>>3;
	Informacion_P_Digital= 			P2IN & 0x7F;
	Informacion_Control=			Informacion_Control^Leer_Puerto;
	Enviar_serial_text(Flag_Inicio);
	Enviar_serial_text(Bits_Direccion);
	Enviar_serial_text(Informacion_Control);
	Enviar_serial_text(Informacion_P_Digital);
	Bits_Crc_Calculado= CRCTrama(Informacion_P_Digital,Informacion_Control,Bits_Direccion);
	Enviar_serial_text(Bits_Crc_Calculado);
	Enviar_serial_text(Flag_Fin);
}
void Metodo_Leer_ADC(){
	if(Bits_Direccion==Longitud+2){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversi
			 Auxiliar_ADC=  ADC12MEM0>>4;
		 }else if(Bits_Direccion==Longitud+3){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM1>>4;
		 }else if(Bits_Direccion== Longitud+4){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM2>>4;
		 }else if(Bits_Direccion==Longitud+5){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM3>>4;
		 }else if(Bits_Direccion==Longitud+6){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM4>>4;
		 }else if(Bits_Direccion==Longitud+7){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM5>>4;
		 }else if(Bits_Direccion==Longitud+8){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM6>>4;
		 }else if(Bits_Direccion==Longitud+9){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM7>>4;
		 }
	Enviar_serial_text(Flag_Inicio);
	Enviar_serial_text(Bits_Direccion);
	Enviar_serial_text(Bits_Control);
	Enviar_serial_text(Auxiliar_ADC);
	Bits_Crc_Calculado= CRCTrama(Auxiliar_ADC,Bits_Control,Bits_Direccion);
	Enviar_serial_text(Bits_Crc_Calculado);
	Enviar_serial_text(Flag_Fin);
	}

void Metodo_Habilitar(){

	if(Bits_Direccion==Longitud+1){

		Bit_Habilitar[1]= true;
	}else if(Bits_Direccion==Longitud+2){
		Bit_Habilitar[2]= true;
	 }else if(Bits_Direccion==Longitud+3){
		 Bit_Habilitar[3]= true;
	 }else if(Bits_Direccion==Longitud+4){
		 Bit_Habilitar[4]= true;
	 }else if(Bits_Direccion==Longitud+5){
		 Bit_Habilitar[5]= true;
	 }else if(Bits_Direccion==Longitud+6){
		 Bit_Habilitar[6]= true;
	 }else if(Bits_Direccion==Longitud+7){
		 Bit_Habilitar[7]= true;
	 }else if(Bits_Direccion==Longitud+8){
		 Bit_Habilitar[8]= true;
	 }else if(Bits_Direccion==Longitud+9){
		 Bit_Habilitar[9]= true;
	 }
}
void Metodo_cambios(){
for (m = 0; m < 10; m++) {
			 if((m==1)&&(Bit_Habilitar[1]==true)){
				 P1OUT^=0x01;
				// Preparando la Trama
				Auxiliar_Leer_Puerto= 			inf[1]& 0x80;
				Informacion_Control= 			Auxiliar_Leer_Puerto>>3;
				Informacion_P_Digital= 			inf[1] & 0x7F;
				Informacion_Control=			Informacion_Control^Leer_Puerto;
				// Envio de la trama
				//Auxiliar_Leer_Puerto= inf[1];
				Enviar_serial_text(Flag_Inicio);
				Enviar_serial_text(Longitud+1);
				Enviar_serial_text(Informacion_Control);
				Enviar_serial_text(Informacion_P_Digital);
				Bits_Crc_Calculado= CRCTrama(Informacion_P_Digital,Informacion_Control,Longitud+1);
				Enviar_serial_text(Bits_Crc_Calculado);
				Enviar_serial_text(Flag_Fin);
			}else if(Bit_Habilitar[m]==true){

				// Preparando la Trama
				Informacion_Control= 			inf[m] & 0x380;
				Informacion_Control= 			Informacion_Control>>3;
				Informacion_P_ADC= 				inf[m]&0x7f;
				Informacion_Control=Informacion_Control ^ Leer_ADC;
				// Envio de la trama
				Enviar_serial_text(Flag_Inicio);
				Enviar_serial_text(Longitud + m);
				Enviar_serial_text(Informacion_Control);
				Enviar_serial_text(Informacion_P_ADC);
				Bits_Crc_Calculado= CRCTrama(Informacion_P_ADC,Informacion_Control,Longitud + m);
				Enviar_serial_text(Bits_Crc_Calculado);
				Enviar_serial_text(Flag_Fin);
			}
	}
}
void Actualizar_Datos(){						// Muestreo --> 20 ms
	inf[1]= P2IN;
	ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
	//while(ADC12CTL1 & ADC12BUSY);
	while (!(ADC12IFG & BIT0));
	inf[2]= ADC12MEM0>>2;
	while (!(ADC12IFG & BIT1));
	//while(ADC12CTL1 & ADC12BUSY);
	inf[3]= ADC12MEM1>>2;
	while (!(ADC12IFG & BIT2));
	//while(ADC12CTL1 & ADC12BUSY);
	inf[4]= ADC12MEM2>>2;
	while (!(ADC12IFG & BIT3));
	//while(ADC12CTL1 & ADC12BUSY);
	inf[5]= ADC12MEM3>>2;
	while (!(ADC12IFG & BIT4));
	//while(ADC12CTL1 & ADC12BUSY);
	inf[6]= ADC12MEM4>>2;
	while (!(ADC12IFG & BIT5));
	//while(ADC12CTL1 & ADC12BUSY);
	inf[7]= ADC12MEM5>>2;
	while (!(ADC12IFG & BIT6));
	//while(ADC12CTL1 & ADC12BUSY);
	inf[8]= ADC12MEM6>>2;
	while (!(ADC12IFG & BIT7));
	//while(ADC12CTL1 & ADC12BUSY);
	inf[9]= ADC12MEM7>>2;

	for (m = 2; m <10; m++) {
		VFiltros[m].x2 =VFiltros[m].x1;
		VFiltros[m].x1 =VFiltros[m].x0;
		VFiltros[m].y2 =VFiltros[m].y1;
		VFiltros[m].y2 =VFiltros[m].y0;
		VFiltros[m].x0= inf[m];
		VFiltros[m].y0= ((VFiltros[m].x0*a0)+(VFiltros[m].x1*a1)+(VFiltros[m].x2*a2)-(VFiltros[m].y1*b1)-(VFiltros[m].y2*b2))/b0;
		inf[m]= VFiltros[m].y0;
	}
}
//=============================================================================
void Recepcion_Bifrost(){

	if(Captura_rx==Flag_Inicio){
		Flag_Inicio_Trama= true;
		Contador_interrupcion=1;
	}
	if(Flag_Inicio_Trama==true){
		Rx_Bifrost[Contador_interrupcion]= Captura_rx;
	}
	if(Contador_interrupcion==6){
		Flag_Inicio_Trama=false;

		Bits_Direccion  = 	Rx_Bifrost[2];
		Bits_Control    =	Rx_Bifrost[3]&0x0f;
		Bits_Informacion= 	Rx_Bifrost[4]&0x7f;										// toma los 7 bit de menor peso de informacion.
		Bits_Crc=			Rx_Bifrost[5];
		Bits_Crc_Calculado= CRCTrama(Rx_Bifrost[4],Rx_Bifrost[3],Rx_Bifrost[2]);
		//if((Rx_Bifrost[Contador_interrupcion]==Flag_Fin)&&(Bits_Crc==Bits_Crc_Calculado)){
		if(Rx_Bifrost[Contador_interrupcion]==Flag_Fin){
			Contador_interrupcion=0;
			if((Bits_Control== Escribir_Puerto)&&(Bits_Direccion==0x22)){
				Metodo_Escribir_Puerto();
			}else if((Bits_Control== Leer_Puerto)&&(Bits_Direccion==0x23)){
				Metodo_Leer_Puerto();
			}else if(Bits_Control== Leer_ADC){
				Metodo_Leer_ADC();
			}else if(Bits_Control==Configuracion_Inicial){
				Metodo_Habilitar();
			}else if(Bits_Control==Enviar_Cambios){
				Metodo_cambios();
			}
		}
	}
}
//=============================================================================
void main(void) {
     WDTCTL = WDTPW | WDTHOLD;					// Stop watchdog timer
    MyReloj_12Mhz();
    Iniciar_puertos();
    Iniciar_ADC();
    Iniciar_Uart();
    Iniciar_Timer();
    __bis_SR_register(GIE); 					// Enable interrupts Globals
    while(1){

    	if(Bandera_int_Rx==true){
    		Recepcion_Bifrost();
    		Bandera_int_Rx=false;
    	}else if(Bandera_int_Timer==true){
    		Actualizar_Datos();
    		Bandera_int_Timer= false;
    	}
    }
}
// ======================== Zona de interrupcion ==============================
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void){
	Contador_interrupcion+=1;
	Captura_rx=UCA0RXBUF;
	Bandera_int_Rx= true;
}
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void){
	Bandera_int_Timer= true;
	//Actualizar_Datos();
}

//=============================================================================
//=============================================================================
