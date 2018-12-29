#include <msp430.h> 
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "CrcLib.h"
#include "QmathLib.h"
//====================================
volatile unsigned int Contador_interrupcion=0;
volatile unsigned char Captura_rx=NULL;
// Vatiables de Control de trama
unsigned short inf[10];
unsigned char Rx_Bifrost[7];
unsigned char Bits_Control=0;
unsigned char Bits_Direccion=0;
unsigned char Bits_Informacion=0;
unsigned char Bits_Crc=0;
unsigned char Bits_Crc_Calculado=0;
//
unsigned char Auxiliar_Leer_Puerto;
short Auxiliar_ADC;
unsigned short  Informacion_Control, Informacion_P_ADC, Informacion_P_Digital;
// Variables de formato tipo Q, necesarias para la implementacion de los filtros Digitales.
_q4 qA;
// Coeficientes de los filtros Digitales ( PASABAJAS 10 Hz )
#define b0 1
#define b1 -1.1430
#define b2 0.4128
#define a0 0.0675
#define a1 0.1349
#define a2 0.0675

struct ValoresFiltro   //esctructura creada para el calculo de los filtros
{
		signed	short x0;
		signed	short y0;
		signed	short x1;
		signed	short y1;
		signed	short x2;
		signed	short y2;

};
struct ValoresFiltro Vfiltro[10];

//====================================
#define Escribir_Puerto 0x03							// Escribe en el Puerto
#define Leer_Puerto 0x05								// Lee el valor del Puerto
#define Leer_ADC 0x09									// Lee el valor del ADC
#define Configuracion_Inicial 0x0C						// Activa las entradas
#define Desactivado 0x00								// Valor NULL
#define Enviar_Cambios 0x02								// Envia Todos los Datos

//====================================
bool Flag_Inicio_Trama= false;
bool Bandera_int_Rx=false;								// Bandera para capturar Trama
bool Bandera_int_Timer=false;							// Bandera para Actualizar Datos
bool Bit_Habilitar[10];									// Bandera para Habilitar Puertos
char cadena[4];											// Envio de Datos
int m=0;
// BANDERAS IDENTIFICADORAS DE TARJETAS...
#define Flag_Inicio 	0xc8							// Tarjeta definitiva
//#define Flag_Inicio 	0xc9							// Tarjeta definitiva
//#define Flag_Inicio 	0xca							// Tarjeta definitiva
//#define Flag_Inicio 	0xcb							// Tarjeta definitiva
#define Longitud 0x00									// Direccion de tarjeta 2
#define Flag_Fin 		0xFD							// Fin de Trama.
//============ Configuracion UART =================

void Iniciar_Uart(void){
	P3SEL |= BIT3+BIT4; 								// P3.3 P3.4 = TXD/RXD
	UCA0CTL1 |= UCSWRST; 								// Reset the state machine-USCI logic held in reset state.
	UCA0CTL1 |= UCSSEL_2; 								// SMCLK como fuente de reloj
	//UCA0BR0 = 72; 										// 8.3MHz/115200 = 72
	//UCA0BR0 = 106; 										// 12.3MHz/115200 = 106
	UCA0BR0 = 175; 										// 20MHz/115200 = 175, 230400= 87
	UCA0BR1 = 0;
	//UCA0MCTL |=  UCBRF_7; 								// Modulation UCBRSx=7, UCBRFx=0
	//UCA0MCTL |=  UCBRS_1; 								// Modulation UCBRSx=1, UCBRFx=0
	UCA0MCTL |=  UCBRS_5; 								// Modulation UCBRSx=7----5, UCBRFx=0
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
	//TA0CCR0 = 30720;									// se carga para 20 Ms en caso de que el reloj este en 12.3 Mhz
	//TA0CCR0 = 50380;									// se carga para 20 Ms en caso de que el reloj este en 20Mhz
	TA0CCR0 = 25191;									// se carga para 10 Ms en caso de que el reloj este en 20Mhz
	TA0CTL = TASSEL_2 + MC_1 +ID_3;         				// SMCLK, Modo UP, Divi 8.
//============ Configuracion Timer ================
}

void Iniciar_puertos(void){
	P1DIR= 0xFF;   										// Puerto 1 como salidas Digitales
	P2DIR= 0x00;										// Puerto 2 como entradas Digitales
	P5DIR= BIT2;
	P5OUT=0x00;
	P2REN|= 0x7F;										// Se habilita las resistencias de Pull-up
//============ Configuracion Puertos ==============
}
//============ Configuracion Reloj ==================

void MyReloj(void){
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
		UCSCTL2 = 374;                   				// Set DCO Multiplier for 12MHz
		__bic_SR_register(SCG0);
		__delay_cycles(375000);							// Tiempo para establcer el reloj.
}

void MyReloj_20Mhz(void){
		UCSCTL3 = SELREF_2; 							// Set DCO FLL reference = REFO
		UCSCTL4 |= SELA_2; 								// Set ACLK = REFO
		UCSCTL0 = 0x0000; 								// Set lowest possible DCOx, MOD
			do
			{
			UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
			SFRIFG1 &= ~OFIFG;
			}while (SFRIFG1&OFIFG);

		__bis_SR_register(SCG0);
		UCSCTL1 = DCORSEL_6;
		UCSCTL2 |= 615;                   				// Set DCO Multiplier for 20MHz
		__bic_SR_register(SCG0);
		__delay_cycles(700000);							// Tiempo para establcer el reloj.
}

//============ Configuracion Reloj ==================
//============ Enviar Informacion  ==================

void Enviar_serial_text(char c){						// Se realiza el envio de la palabra.
		while (!(UCA0IFG & UCTXIFG));
		UCA0TXBUF =c;
	    	  }
//============ Enviar Informacion  ==================
void Metodo_Escribir_Puerto(){							// Escribe en el puerto.
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
			 Auxiliar_ADC=  ADC12MEM0>>2;
		 }else if(Bits_Direccion==Longitud+3){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM1>>2;
		 }else if(Bits_Direccion== Longitud+4){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM2>>2;
		 }else if(Bits_Direccion==Longitud+5){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM3>>2;
		 }else if(Bits_Direccion==Longitud+6){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM4>>2;
		 }else if(Bits_Direccion==Longitud+7){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM5>>2;
		 }else if(Bits_Direccion==Longitud+8){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM6>>2;
		 }else if(Bits_Direccion==Longitud+9){
			 ADC12CTL0 |= ADC12SC;                   	// Start sampling/conversion
			 Auxiliar_ADC=  ADC12MEM7>>2;
		 }

	Vfiltro[0].x2 = Vfiltro[0].x1;
	Vfiltro[0].x1 = Vfiltro[0].x0;
	Vfiltro[0].y2 = Vfiltro[0].y1;
	Vfiltro[0].y1 = Vfiltro[0].y0;
	Vfiltro[0].x0=  Auxiliar_ADC;
			//Y_0=((X_0*b0)+(X_1*b1)+(X_2*b2)-(Y_1*a1)-(Y_2*a2))/a0; Ecuacion del filtro...
			qA= _Q4mpyI16(_Q4(a0),Vfiltro[0].x0) + _Q4mpyI16(_Q4(a1),Vfiltro[0].x1) + _Q4mpyI16(_Q4(a2),Vfiltro[0].x2) - _Q4mpyI16(_Q4(b1),Vfiltro[0].y1) - _Q4mpyI16(_Q4(b2),Vfiltro[0].y2);
			qA=  _Q4div(qA,_Q4(b0));
			Vfiltro[m].y0=  _Q4toF(qA);
			Vfiltro[m].y0=  Vfiltro[0].y0&0x3FE;
			Auxiliar_ADC= Vfiltro[0].y0;
					Informacion_Control= 			Auxiliar_ADC & 0x380;
					Informacion_Control= 			Informacion_Control>>3;
					Informacion_P_ADC= 				Auxiliar_ADC &0x7f;
					Informacion_Control=			Informacion_Control ^ Leer_ADC;

	Enviar_serial_text(Flag_Inicio);
	Enviar_serial_text(Bits_Direccion);
	Enviar_serial_text(Informacion_Control);
	Enviar_serial_text(Informacion_P_ADC);
	Bits_Crc_Calculado= CRCTrama(Informacion_P_ADC,Informacion_Control,Bits_Direccion);
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

void SimpreHabilitar(){
		 Bit_Habilitar[1]= true;
		 Bit_Habilitar[2]= true;
		 Bit_Habilitar[3]= true;
		 Bit_Habilitar[4]= true;
		 Bit_Habilitar[5]= true;
		 Bit_Habilitar[6]= true;
		 Bit_Habilitar[7]= true;
		 Bit_Habilitar[8]= true;
		 Bit_Habilitar[9]= true;
}
void Metodo_cambios(){
	P5OUT= BIT2;
for (m = 0; m < 10; m++) {																		
			 if((m==1)&&(Bit_Habilitar[1]==true)){
				// Preparando la Trama
				Auxiliar_Leer_Puerto= 			inf[1]& 0x80;
				Informacion_Control= 			Auxiliar_Leer_Puerto>>3;
				Informacion_P_Digital= 			inf[1] & 0x7F;
				Informacion_Control=			Informacion_Control^Leer_Puerto;
				// Envio de la trama
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
// Trama de Fin de los datos actualizados...
Enviar_serial_text(Flag_Inicio);
Enviar_serial_text(0xc3);
Enviar_serial_text(0xc3);
Enviar_serial_text(0xc3);
Enviar_serial_text(0xc3);
Enviar_serial_text(Flag_Fin);
while (UCA0STAT & UCBUSY) ;                      // Wait for TX complete
	//__delay_cycles(500);
P5OUT= 0x00;

}
void Actualizar_Datos(){
	inf[1]= P2IN&0x7f;
	ADC12CTL0 |= ADC12SC;
	while (!(ADC12IFG & BIT0));
	inf[2]= ADC12MEM0>>2;
	__delay_cycles(10);
	while (!(ADC12IFG & BIT1));
	inf[3]= ADC12MEM1>>2;
	__delay_cycles(10);
	while (!(ADC12IFG & BIT2));
	inf[4]= ADC12MEM2>>2;
	__delay_cycles(10);
	while (!(ADC12IFG & BIT3));
	inf[5]= ADC12MEM3>>2;
	__delay_cycles(10);
	while (!(ADC12IFG & BIT4));
	inf[6]= ADC12MEM4>>2;
	while (!(ADC12IFG & BIT5));
	inf[7]= ADC12MEM5>>2;
	while (!(ADC12IFG & BIT6));
	inf[8]= ADC12MEM6>>2;
	while (!(ADC12IFG & BIT7));
	inf[9]= ADC12MEM7>>2;
	for (m = 2; m <10; m++) {
		Vfiltro[m].x2 = Vfiltro[m].x1;
		Vfiltro[m].x1 = Vfiltro[m].x0;
		Vfiltro[m].y2 = Vfiltro[m].y1;
		Vfiltro[m].y1 = Vfiltro[m].y0;
		Vfiltro[m].x0=  inf[m];
				//Y_0=((X_0*b0)+(X_1*b1)+(X_2*b2)-(Y_1*a1)-(Y_2*a2))/a0; Ecuacion del filtro...
				qA= _Q4mpyI16(_Q4(a0),Vfiltro[m].x0) + _Q4mpyI16(_Q4(a1),Vfiltro[m].x1) + _Q4mpyI16(_Q4(a2),Vfiltro[m].x2) - _Q4mpyI16(_Q4(b1),Vfiltro[m].y1) - _Q4mpyI16(_Q4(b2),Vfiltro[m].y2);
				qA=  _Q4div(qA,_Q4(b0));
				Vfiltro[m].y0=  _Q4toF(qA);
				Vfiltro[m].y0=  Vfiltro[m].y0&0x3FE;
				inf[m]= Vfiltro[m].y0;
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
		//if((Rx_Bifrost[Contador_interrupcion]==Flag_Fin)&&(Bits_Crc==Bits_Crc_Calculado)){
		if((Rx_Bifrost[6]==Flag_Fin)&&(Rx_Bifrost[1]==Flag_Inicio )){
			Bits_Direccion  = 	Rx_Bifrost[2];
			Bits_Control    =	Rx_Bifrost[3]&0x0f;
			Bits_Informacion= 	Rx_Bifrost[4]&0x7f;										// toma los 7 bit de menor peso de informacion.
			Bits_Crc=			Rx_Bifrost[5];
			Bits_Crc_Calculado= CRCTrama(Rx_Bifrost[4],Rx_Bifrost[3],Rx_Bifrost[2]);
			Contador_interrupcion=0;
			for(m=1;m<=6;m++){
				Rx_Bifrost[m]=0;
			}
			if((Bits_Control== Escribir_Puerto)&&(Bits_Direccion==Longitud)){
				Metodo_Escribir_Puerto();
			}else if((Bits_Control== Leer_Puerto)&&(Bits_Direccion==Longitud+1)){
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
    //MyReloj();
    MyReloj_20Mhz();
    Iniciar_puertos();
    Iniciar_ADC();
    Iniciar_Uart();
    Iniciar_Timer();
    SimpreHabilitar();
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
