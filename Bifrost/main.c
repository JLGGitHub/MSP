
//--------------------------
//BIOS header files
//--------------------------
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/cfg/global.h>
#include <ti/sysbios/knl/Task.h>
#include <xdc/runtime/Log.h>
//----------------------------
//TivaWare Header FIles
//----------------------------
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "utils/uartstdio.h"
#include "driverlib/systick.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
////#include "inc/tm4c123gh6pm.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"

#include  "CrcLib.h"

//-----------------------------
// Macros del codigo
//-----------------------------

#define Length	 			  16 //longitud 16
#define ChannelAdc 			  10 // canales_adc 10
#define ChannelPwm 			   8 // canales_pwm 8
#define StartFlagPcToArm 	 129 //inicio_trama_software 129   // Es la misma dirección de la tarjeta 0x81   10000001
#define StartFlagFpgaToArm   252 //inicio_trama_fpga_arm  252
#define FrameEndFlag 		 253 //fin_de_trama 253


// FRECUENCIAS DE PWM
#define Frec_FPGA   5000000   //FRECUENCIA FPGA   PD6  5000000 11520000
#define Frec_PWM0   1000     //FRECUENCIA PWM0
#define Frec_PWM1   1000     //FRECUENCIA PWM1
#define Frec_PWM2   3000     //FRECUENCIA PWM2
#define Frec_PWM3   4000     //FRECUENCIA PWM3
#define Frec_PWM4   50    //FRECUENCIA PWM4     ---Permite bajas frecuencias
#define Frec_PWM5   50     //FRECUENCIA PWM5    ---Permite bajas frecuencias
#define Frec_PWM6   1000     //FRECUENCIA PWM6  ---Permite bajas frecuencias
#define Frec_PWM7   1000    //FRECUENCIA PWM7   ---Permite bajas frecuencias

//coeficientes del filtro digital de 10Hz
#define b0 1   //Valores para filtro pasabajas de 10Hz 50 muestras por segundo
#define b1 -1.1430
#define b2 0.4128
#define a0 0.0675
#define a1 0.1349
#define a2 0.0675

//-----------------------------
// Funciones o Tareas creadas
//-----------------------------
void InitGlobalVariables(void);
void HardwareInit(void);

void IntUart0(void);
	void TaskGetDataFramePcToArm(void);
		void TaskSendAllDataTable(void);  //tareas llamadas dentro de otras tareas
		void TaskSendChangedDataTable(void);
		void TaskConfigureDataTable(void);
		void TaskWriteDataTable (void);
			void TaskWriteFpga(void);
			void TaskWritePwm(void);

void IntUart4(void);
	void TaskGetFrameFpgaToArm(void);
		void TaskWriteSecondDataTable(void);
			void TaskCompareDataTables(void);

//---------------------------------------------------------\\
//		 TASK PERIODS
//---------------------------------------------------------\\
void TaskReadDigitalInputPort(void);

void ClockTaskReadAdc(void);
void TaskReadAdc0(void);
void TaskReadAdc1(void);
//Funciones llamadas por tareas
//void WriteFpga(char Address);
void PWMPORT(unsigned long ulDire, unsigned long ulWidth );
//void EnableAdcModules(short sAddress);

//----------------------------
//Variables y constantes Define
//----------------------------
enum ControlWord
{
	DisablePort=0,
	WritePort=3,
	PortDeclaredAsRead=5,
	WriteDac=6,
	EnableAdc=9,
	EnableWritePwm=11,
	ConfigInit=12,
	SendAllDataTable=193,
	SendChangedDataTable=194
} ;


struct BaseDatos
{
	char address;
	char control;
	char information;
};

struct ValoresFiltro   //esctructura creada para el calculo de los filtros
{
	float X0;
	float Y0;
	float X1;
	float Y1;
	float X2;
	float Y2;

};

struct BaseDatos 	 PrimaryDataTable[Length+ChannelAdc+ChannelPwm];  // base_datos_principal Tabla principal donde se almacena los datos
struct BaseDatos 	 SecondDataTable [Length+ChannelAdc];  //Tabla secundaria y almacena los datos provenientes de la fpga
struct ValoresFiltro valor_pas_pre[ChannelAdc];           //contiene los valores presentes y pasados necesarios para el calculo de filtros

unsigned char ucGetUart0;						//CAPTURA=15
unsigned char ucGetUart4;						//receptor_uart_4
bool 		  bGetTrama     = false;				//ENVIA
bool          bGetTramaFpga = false;              //bandera_uart_4
bool          bInitModAdc0=true;
bool          bInitModAdc1=true;
bool          bInitGpioE=true;


short         sCounterArm	= 0;					//cuenta=0
short 		  sCounterFpga	= 0;       			//cuenta_uart_4=0;


unsigned char ucGetFrameFromPc[6];				//trama_recibida_uart0[6];
unsigned char ucGetFrameFromFpga[6];			//trama_recibida_uart4[6];
bool          bChangeDataPrimaryTable[Length];	//cambio_datos_tabla_principal
bool          bChangeChannelPwm[ChannelPwm];  	//cambio_pwm[canales_pwm];
bool          bChange[Length];         			//cambio[longitud];
float    	  fNewPulseWidth[ChannelPwm];		//usvWidth[8];
unsigned long ulPeriod[8];


void main(void)
{
	InitGlobalVariables();
	HardwareInit();
    BIOS_start();
}
void InitGlobalVariables(void)
{
	int i;
	for(i=0; i<(Length+ChannelAdc+ChannelPwm); i++)
	{
		PrimaryDataTable[i].address = i+1;
		PrimaryDataTable[i].control = 0;
		PrimaryDataTable[i].information = 0;
	}
	for(i=0; i<(Length); i++)
	{
		SecondDataTable[i].address = i+1;
		SecondDataTable[i].control = 0;
		SecondDataTable[i].information = 0;
	}
	for(i=0;i<Length;i++)
	{
		 bChange[i] = false;
	}
	for(i=0;i<ChannelAdc;i++)
	{
		valor_pas_pre[i].X0=0;
		valor_pas_pre[i].X1=0;
		valor_pas_pre[i].Y0=0;
		valor_pas_pre[i].Y1=0;
	}
	for(i=0;i<ChannelPwm;i++)
	{
		bChangeChannelPwm[i] = false;
	}
	for(i=0;i<ChannelPwm;i++)
		{
		fNewPulseWidth[i] = 0;
		}
}
void HardwareInit(void)
{
	unsigned long ulClockFPGA;

	SysCtlClockSet(SYSCTL_SYSDIV_2_5| SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	//---------------------------------------------------
	//        configuracion frecuencia pwm
	//---------------------------------------------------
		 ulClockFPGA = (SysCtlClockGet()/ Frec_FPGA)-1;
		 ulPeriod[0] = (SysCtlClockGet()/  Frec_PWM0)-1;
		 ulPeriod[1] = (SysCtlClockGet() / Frec_PWM1)-1;
		 ulPeriod[2] = (SysCtlClockGet() / Frec_PWM2)-1;
		 ulPeriod[3] = (SysCtlClockGet() / Frec_PWM3)-1;
		 ulPeriod[4] = (SysCtlClockGet() / Frec_PWM4)-1;
		 ulPeriod[5] = (SysCtlClockGet() / Frec_PWM5)-1;
		 ulPeriod[6] = (SysCtlClockGet() / Frec_PWM6)-1;
		 ulPeriod[7] = (SysCtlClockGet() / Frec_PWM7)-1;

	//----------------------------------------------------
	//                       PWM FPGA
	//----------------------------------------------------

			SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
			GPIOPinConfigure(GPIO_PD6_WT5CCP0);
			GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_6);


			 // Configure timer
			SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER5);
			TimerConfigure(WTIMER5_BASE, (TIMER_CFG_SPLIT_PAIR |TIMER_CFG_A_PWM));
			TimerControlLevel(WTIMER5_BASE, TIMER_A, 1);

			TimerLoadSet (WTIMER5_BASE, TIMER_A, ulClockFPGA);
			TimerMatchSet(WTIMER5_BASE, TIMER_A, (ulClockFPGA * 0.50));
			TimerEnable  (WTIMER5_BASE, TIMER_A);

		//----------------------------------------------------
			//                       PWM0-	PWM1
			//----------------------------------------------------
				SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
				GPIOPinConfigure(GPIO_PB0_T2CCP0);
				GPIOPinConfigure(GPIO_PB1_T2CCP1);
				GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_0);
				GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_1);


				SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
				TimerConfigure(TIMER2_BASE, (TIMER_CFG_SPLIT_PAIR |TIMER_CFG_B_PWM |TIMER_CFG_A_PWM));
				TimerControlLevel(TIMER2_BASE, TIMER_BOTH, 1);

			    TimerLoadSet (TIMER2_BASE, TIMER_A, ulPeriod[0]);                   //PWM0
				TimerLoadSet (TIMER2_BASE, TIMER_B, ulPeriod[1]);                   //PWM1
				TimerEnable  (TIMER2_BASE, TIMER_BOTH);

			//----------------------------------------------------
			//                       PWM2-PWM3
			//----------------------------------------------------

				SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
				GPIOPinConfigure(GPIO_PB2_T3CCP0);
				GPIOPinConfigure(GPIO_PB3_T3CCP1);
				GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_2);
				GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_3);


				SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
				TimerConfigure(TIMER3_BASE, (TIMER_CFG_SPLIT_PAIR |TIMER_CFG_B_PWM |TIMER_CFG_A_PWM));
				TimerControlLevel(TIMER3_BASE, TIMER_BOTH, 1);

				TimerLoadSet (TIMER3_BASE, TIMER_A, ulPeriod[2]);                   //PWM2
				TimerLoadSet (TIMER3_BASE, TIMER_B, ulPeriod[3]);                   //PWM3
				TimerEnable  (TIMER3_BASE, TIMER_BOTH);

			//----------------------------------------------------
			//                       PWM4-PMW5
			//----------------------------------------------------

				SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
				GPIOPinConfigure(GPIO_PC6_WT1CCP0);
				GPIOPinConfigure(GPIO_PC7_WT1CCP1);
				GPIOPinTypeTimer(GPIO_PORTC_BASE, GPIO_PIN_6);
				GPIOPinTypeTimer(GPIO_PORTC_BASE, GPIO_PIN_7);

				 // Configure timer
				 SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER1);
				 TimerConfigure(WTIMER1_BASE, (TIMER_CFG_SPLIT_PAIR |TIMER_CFG_B_PWM |TIMER_CFG_A_PWM));
				 TimerControlLevel(WTIMER1_BASE, TIMER_BOTH, 1);

				 TimerLoadSet (WTIMER1_BASE, TIMER_A, ulPeriod[4]);                   //PWM4
				 TimerLoadSet (WTIMER1_BASE, TIMER_B, ulPeriod[5]);                   //PWM5
				 TimerEnable  (WTIMER1_BASE, TIMER_BOTH);

			//----------------------------------------------------
			//                       PWM6-PMW7
			//----------------------------------------------------

				 SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
				 GPIOPinConfigure(GPIO_PD4_WT4CCP0);
				 GPIOPinConfigure(GPIO_PD5_WT4CCP1);
				 GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_4);
				 GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_5);

					 // Configure timer
				 SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER4);
				 TimerConfigure(WTIMER4_BASE, (TIMER_CFG_SPLIT_PAIR |TIMER_CFG_B_PWM |TIMER_CFG_A_PWM));
				 TimerControlLevel(WTIMER4_BASE, TIMER_BOTH, 1);

				 TimerLoadSet (WTIMER4_BASE, TIMER_A, ulPeriod[6]);                   //PWM4
				 TimerLoadSet (WTIMER4_BASE, TIMER_B, ulPeriod[7]);                   //PWM5
				 TimerEnable  (WTIMER4_BASE, TIMER_BOTH);

	//----------------------------------------------------
	//                       END-PMW
	//----------------------------------------------------
//=========================================================\\
// CONFIG adc
//=========================================================\\

				 SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
				 SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);




				 SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
				 GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_4);////---A9
				 GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);////---A8
				 GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_0);////---A7
				 GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_1);////---A6
				 GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_2);////---A5

				 SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
				 GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_3);////---A4
				 GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);////---A3
				 GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);////---A2
				 GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);////---A1
				 GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);////---A0

	//------------------------------------------//
		//  	 Configuraciòn del uart0
		//------------------------------------------//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	//------------------------------------------//
	//  	 Configuraciòn del uart4
	//------------------------------------------//
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART4);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    GPIOPinConfigure(GPIO_PC4_U4RX);
    GPIOPinConfigure(GPIO_PC5_U4TX);
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    UARTConfigSetExpClk(UART4_BASE, SysCtlClockGet(), 115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

  	//------------------------------------------//
	// Habilitaciòn de las Interrucciones por Uart
	//------------------------------------------//


    IntMasterEnable();    ///HABILITA LAS INTERRUPCIONES A NIVEL GLOBAL
	IntEnable(INT_UART0);
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
	IntEnable(INT_UART4);
	UARTIntEnable(UART4_BASE, UART_INT_RX | UART_INT_RT);
	UARTFIFODisable(UART4_BASE);
}

//-------------------------------------------\\
//         COMMUNICATION  PC TO ARM
//-------------------------------------------\\

void IntUart0(void)
{
	unsigned int uiStatus;
	uiStatus = UARTIntStatus(UART0_BASE, true); //get interrupt status
    UARTIntClear(UART0_BASE, uiStatus); //clear the asserted interrupts

    if(UARTCharsAvail(UART0_BASE))
    {
    	ucGetUart0 = UARTCharGetNonBlocking(UART0_BASE);
    	Semaphore_post(SmphTaskGetDataFramePcToArm);
    }
}
void TaskGetDataFramePcToArm()
{
	unsigned char ucControl;
	unsigned char ucDataCrc;

	while(1)
	{

		Semaphore_pend(SmphTaskGetDataFramePcToArm, BIOS_WAIT_FOREVER);

     	if(ucGetUart0 == StartFlagPcToArm)
			{
				bGetTrama = true;
			}
		if(bGetTrama == true)
			{
				ucGetFrameFromPc[sCounterArm] = ucGetUart0;
				sCounterArm++;
			}

		ucControl  = ucGetFrameFromPc[2];

			if(sCounterArm >= 6)
			{
				sCounterArm= 0;
				bGetTrama= false;

				ucDataCrc = CRCTrama(ucGetFrameFromPc[3],ucGetFrameFromPc[2], ucGetFrameFromPc[1]); //calcular el crc

				if((ucDataCrc==ucGetFrameFromPc[4])&&(FrameEndFlag == ucGetFrameFromPc[5]))
				{

					if(ucControl == SendAllDataTable) //hex C1 decimal 193
					{
						 Semaphore_post(SmphTaskSendAllDataTable);
					}
					else if(ucControl== SendChangedDataTable) //hex C2 decimal 194
					{
						Semaphore_post(SmphTaskSendChangedDataTable);
					}
					else if (ucControl==ConfigInit)
					{
						Semaphore_post(SmphTaskConfigureDataTable);
					}
					else
					{
						Semaphore_post(SmphTaskWriteDataTable);
					}


			    }
			}
	}

}
void TaskSendAllDataTable(void)
{
	char cDataCrcOut;
	int i;
	while(1)
		{
			Semaphore_pend(SmphTaskSendAllDataTable, BIOS_WAIT_FOREVER); // Se desactiva la tarea y espera por nuevos datos

				for(i=0; i<(Length+ChannelAdc+ChannelPwm); i++)  //cambiar
				{
					UARTCharPut(UART0_BASE,StartFlagPcToArm );
					UARTCharPut(UART0_BASE,PrimaryDataTable[i].address);
					UARTCharPut(UART0_BASE,PrimaryDataTable[i].control);
					UARTCharPut(UART0_BASE,PrimaryDataTable[i].information);
					cDataCrcOut =  CRCTrama(PrimaryDataTable[i].information, PrimaryDataTable[i].control,PrimaryDataTable[i].address);
					UARTCharPut(UART0_BASE,cDataCrcOut);                             //lugar CRC
					UARTCharPut(UART0_BASE,FrameEndFlag);                          //base_datos_secundaria

				}
		}
	}
void TaskSendChangedDataTable(void)
{
	int iControl;
	int i;
	int iNumberChanges;
	char cDataCrcOut;
		while(1)
		{
			Semaphore_pend(SmphTaskSendChangedDataTable, BIOS_WAIT_FOREVER);

			for(i=0;i<(Length-4);i++)
			{
				if(bChange[i] == true)/// se elimina y se pone PrimaryDataTable[sPositionChangesFpga[sCountFpgaWrite]] otra    variable iAux=sPositionChangesFpga[sCountFpgaWrite]-->PrimaryDataTable[iAux]
				{
					UARTCharPut(UART0_BASE,StartFlagPcToArm);
					UARTCharPut(UART0_BASE,PrimaryDataTable[i].address);
					UARTCharPut(UART0_BASE,PrimaryDataTable[i].control);
					UARTCharPut(UART0_BASE,PrimaryDataTable[i].information);
					cDataCrcOut=  CRCTrama(PrimaryDataTable[i].information, PrimaryDataTable[i].control, PrimaryDataTable[i].address);
					UARTCharPut(UART0_BASE,cDataCrcOut);                             //lugar CRC
					UARTCharPut(UART0_BASE,FrameEndFlag);
					bChange[i] = false;
					iNumberChanges++;
				}
			}


			for(i=Length; i<=(Length+ChannelAdc);i++)
			{
				iControl = PrimaryDataTable[i].control & 15;
				if(iControl == EnableAdc)
				{
					UARTCharPut(UART0_BASE,StartFlagPcToArm);
					UARTCharPut(UART0_BASE,PrimaryDataTable[i].address);
					UARTCharPut(UART0_BASE,PrimaryDataTable[i].control);
					UARTCharPut(UART0_BASE,PrimaryDataTable[i].information);
					cDataCrcOut=  CRCTrama(PrimaryDataTable[i].information, PrimaryDataTable[i].control, PrimaryDataTable[i].address);
					UARTCharPut(UART0_BASE,cDataCrcOut);                      //lugar CRC
					UARTCharPut(UART0_BASE,FrameEndFlag);
					iNumberChanges++;
				}
			}

			if(iNumberChanges == 0)
				{
					UARTCharPut(UART0_BASE,StartFlagPcToArm);
					UARTCharPut(UART0_BASE,0);
					UARTCharPut(UART0_BASE,0);
					UARTCharPut(UART0_BASE,0);
					cDataCrcOut =  CRCTrama(0,0,0);
					UARTCharPut(UART0_BASE,cDataCrcOut);                             //lugar CRC
					UARTCharPut(UART0_BASE,FrameEndFlag);
				}
		}

}
void TaskConfigureDataTable(void)
{
	char cAddress;
	char cControl=12;
	char cDataCrcOut;
		int i;

	while(1)
			{

				Semaphore_pend(SmphTaskConfigureDataTable,BIOS_WAIT_FOREVER);
				cAddress    = ucGetFrameFromPc[1];
				PrimaryDataTable[cAddress].address= ucGetFrameFromPc[1];
				PrimaryDataTable[cAddress].control = ucGetFrameFromPc[3];


			if ((cAddress)>=33)
			{
				for(i=0; i<(Length+ChannelAdc+ChannelPwm); i++)  //cambiar
								{
									UARTCharPut(UART0_BASE,StartFlagPcToArm );
									UARTCharPut(UART0_BASE,PrimaryDataTable[i].address);//direccion
									UARTCharPut(UART0_BASE,cControl);//control
									UARTCharPut(UART0_BASE,PrimaryDataTable[i].control&15);//informacion
									cDataCrcOut =  CRCTrama(PrimaryDataTable[i].control&15,12,PrimaryDataTable[i].address);///crc
									UARTCharPut(UART0_BASE,cDataCrcOut);                             //lugar CRC
									UARTCharPut(UART0_BASE,FrameEndFlag);                          //base_datos_secundaria
									PrimaryDataTable[i].information=0;
								}



			/*	for(i=0; i<Length; i++)     /// for de  leerr posiciones a enviar
										{
					    	// 				cControl = PrimaryDataTable[(sPositionChanges[i])].control & 15;
								           cControl = PrimaryDataTable[i].control & 15;

								           if(cControl == WritePort)
								            {
								            	//if(bChangeDataPrimaryTable[i]==true)
								            	//{
									            	UARTCharPut(UART4_BASE,StartFlagFpgaToArm);
													UARTCharPut(UART4_BASE,PrimaryDataTable[i].address+1);
													UARTCharPut(UART4_BASE,PrimaryDataTable[i].control);
													UARTCharPut(UART4_BASE,PrimaryDataTable[i].information);
													UARTCharPut(UART4_BASE,114);                             //lugar CRC
													UARTCharPut(UART4_BASE,FrameEndFlag);
													//bChangeDataPrimaryTable[i] = false;
								            	//}
								            }
								           if(cControl == PortDeclaredAsRead)// lectura de las variables del puerto de entrada
								            {
								            	UARTCharPut(UART4_BASE,StartFlagFpgaToArm);
												UARTCharPut(UART4_BASE,PrimaryDataTable[i].address+1);
												UARTCharPut(UART4_BASE,(PrimaryDataTable[i].control));
												UARTCharPut(UART4_BASE,PrimaryDataTable[i].information);
												UARTCharPut(UART4_BASE,114);                             //lugar CRC
												UARTCharPut(UART4_BASE,FrameEndFlag);
								            }
								           if(cControl == WriteDac)
								            {
								            		UARTCharPut(UART4_BASE,StartFlagFpgaToArm);
													UARTCharPut(UART4_BASE,(PrimaryDataTable[i].address - 12));
													UARTCharPut(UART4_BASE,PrimaryDataTable[i].control);
													UARTCharPut(UART4_BASE,PrimaryDataTable[i].information);
													UARTCharPut(UART4_BASE,114);                             //lugar CRC
													UARTCharPut(UART4_BASE,FrameEndFlag);
								            }
										}*/


			}
			}
}
void TaskWriteDataTable (void)
{
	char cAddress;
	char cControl;
	char cInformation;

		while(1)
		{
			Semaphore_pend(SmphTaskWriteDataTable, BIOS_WAIT_FOREVER);  // Se desactiva la tarea y espera por nuevos datos
			cAddress    = ucGetFrameFromPc[1];
			cControl     = ucGetFrameFromPc[2] & 15;
			cInformation = ucGetFrameFromPc[3];

			if (cControl == WritePort || cControl == PortDeclaredAsRead || cControl== WriteDac)//Indica que el puerto funcionará como una 5entrada/3salida digital.  6 habilita y escribe en el Dac
		    {
				PrimaryDataTable[cAddress].address= ucGetFrameFromPc[1];
				PrimaryDataTable[cAddress].control = ucGetFrameFromPc[2];
		    	PrimaryDataTable[cAddress].information = cInformation;
		    	bChangeDataPrimaryTable[cAddress]=true;
		    }
		    else if (cControl == DisablePort)  //deshabilitar puerto o canal ADC
		   	    {
		    	    PrimaryDataTable[cAddress].address= ucGetFrameFromPc[1];
		    	    PrimaryDataTable[cAddress].control = ucGetFrameFromPc[2];
		    		PrimaryDataTable[cAddress].information = cInformation;
		   	    }
		    else if (cControl == EnableAdc)    //habilitar canal adc //// ver que variable va con o sin emascarar
		    	   	    {
		    	            PrimaryDataTable[cAddress].address= ucGetFrameFromPc[1];
		    				PrimaryDataTable[cAddress].control = ucGetFrameFromPc[2];
		    				PrimaryDataTable[cAddress].information = cInformation;
		    				//EnableAdcModules(cAddress-16);
		    	   	    }
		    else if(cControl == EnableWritePwm)        //canales pwm
		    {
		    	PrimaryDataTable[cAddress].address= ucGetFrameFromPc[1];
		    	PrimaryDataTable[cAddress].control = ucGetFrameFromPc[2];
		    	PrimaryDataTable[cAddress].information = cInformation;
		       // cControl = cAddress - 27;
		        bChangeChannelPwm[(cAddress - 26)] = true;//// escribe en la estructura la posicion a Enviar PWM

		    }
		}
}


//-------------------------------------------\\
//         COMMUNICATION  ARM TO FPGA
//-------------------------------------------\\

void IntUart4(void)
{
	unsigned int uiStatus_2;
	uiStatus_2 =  UARTIntStatus(UART4_BASE, true);
	UARTIntClear(UART4_BASE, uiStatus_2);
	if(UARTCharsAvail(UART4_BASE))
	{
	    ucGetUart4 =  UARTCharGetNonBlocking(UART4_BASE);     // UARTCharPut(UART0_BASE, UARTCharGet(UART4_BASE));
	    Semaphore_post(SmphTaskGetFrameFpgaToArm);
	}
}
void TaskGetFrameFpgaToArm(void)
{
	while(1){
				Semaphore_pend(SmphTaskGetFrameFpgaToArm, BIOS_WAIT_FOREVER);

				if(ucGetUart4 == StartFlagFpgaToArm)
					{
						bGetTramaFpga = true;
					}
				if(bGetTramaFpga == true)
					{
						ucGetFrameFromFpga[sCounterFpga] = ucGetUart4;
						sCounterFpga++;
					}
				if(sCounterFpga == 6)
					{
						sCounterFpga  = 0;
						bGetTramaFpga = false;

						if(FrameEndFlag == ucGetFrameFromFpga[5])
						{
							Semaphore_post(SmphTaskWriteSecondDataTable);
						}
					}
		   }
}
void TaskWriteSecondDataTable(void)
{
	char cAddress, cControl, cInformation;
		while(1)
		{
			Semaphore_pend(SmphTaskWriteSecondDataTable, BIOS_WAIT_FOREVER); // Se desactiva la tarea y espera por nuevos datos

			cAddress = ucGetFrameFromFpga[1] - 1;
			cControl = ucGetFrameFromFpga[2];
			cInformation = ucGetFrameFromFpga[3];
			if (cControl >= WritePort && cControl <= PortDeclaredAsRead)
				    {
						SecondDataTable[cAddress].address= cAddress;
						SecondDataTable[cAddress].control = cControl;
						SecondDataTable[cAddress].information = cInformation;
						//Semaphore_post(SmphTaskCompareDataTables);//llamar tarea de comparar tablas
				    }
		}
}


//-------------------------------------------\\
//         CLOCK READ DIGITAL INPUT PORT
//-------------------------------------------\\


void TaskReadDigitalInputPort(void)
{

	int i=0;
	while(1)
	{

		Semaphore_pend(SmphTaskReadDigitalInputPort, 53.13);

		for(i=0;i<12;i++)
		{
			if(PrimaryDataTable[i].control ==PortDeclaredAsRead)
			{
				UARTCharPut(UART4_BASE,StartFlagFpgaToArm);
				UARTCharPut(UART4_BASE,PrimaryDataTable[i].address+1);
				UARTCharPut(UART4_BASE,PrimaryDataTable[i].control-1);
				UARTCharPut(UART4_BASE,PrimaryDataTable[i].information);
				UARTCharPut(UART4_BASE,114); //lugar CRC
				UARTCharPut(UART4_BASE,FrameEndFlag);
			}
		}

	}
}
void TaskCompareDataTables(void)
{
	int i;
	while(1)
		{
			Semaphore_pend(SmphTaskCompareDataTables, 66.661);

		  for(i=0;i<(Length-4);i++)
			{
				if(PrimaryDataTable[i].control == PortDeclaredAsRead)
				{
					if(SecondDataTable[i].information != PrimaryDataTable[i].information)
					{
						PrimaryDataTable[i].information = SecondDataTable[i].information;
						bChange[i] = true;
					}
				}
			}
		}
}

//-------------------------------------------\\
//         CLOCK WRITE FPGA PORT
//-------------------------------------------\\


void TaskWriteFpga(void)
{
	int i;
	char cControl;
	while(1)
		{

		Semaphore_pend(SmphTaskWriteFpga, 50.55);

	    for(i=0; i<Length; i++)     /// for de  leerr posiciones a enviar
						{
	    	// 				cControl = PrimaryDataTable[(sPositionChanges[i])].control & 15;
				           cControl = PrimaryDataTable[i].control & 15;

				           if(cControl == WritePort)
				            {
				            	//if(bChangeDataPrimaryTable[i]==true)
				            	//{
					            	UARTCharPut(UART4_BASE,StartFlagFpgaToArm);
									UARTCharPut(UART4_BASE,PrimaryDataTable[i].address+1);
									UARTCharPut(UART4_BASE,PrimaryDataTable[i].control);
									UARTCharPut(UART4_BASE,PrimaryDataTable[i].information);
									UARTCharPut(UART4_BASE,114);                             //lugar CRC
									UARTCharPut(UART4_BASE,FrameEndFlag);
									//bChangeDataPrimaryTable[i] = false;
				            	//}
				            }
				           if(cControl == PortDeclaredAsRead)// lectura de las variables del puerto de entrada
				            {
				            	UARTCharPut(UART4_BASE,StartFlagFpgaToArm);
								UARTCharPut(UART4_BASE,PrimaryDataTable[i].address+1);
								UARTCharPut(UART4_BASE,(PrimaryDataTable[i].control));
								UARTCharPut(UART4_BASE,PrimaryDataTable[i].information);
								UARTCharPut(UART4_BASE,114);                             //lugar CRC
								UARTCharPut(UART4_BASE,FrameEndFlag);
				            }
				           if(cControl == WriteDac)
				            {
				            		UARTCharPut(UART4_BASE,StartFlagFpgaToArm);
									UARTCharPut(UART4_BASE,(PrimaryDataTable[i].address - 12));
									UARTCharPut(UART4_BASE,PrimaryDataTable[i].control);
									UARTCharPut(UART4_BASE,PrimaryDataTable[i].information);
									UARTCharPut(UART4_BASE,114);                             //lugar CRC
									UARTCharPut(UART4_BASE,FrameEndFlag);
				            }
						}
			}
}
void TaskWritePwm(void)//Pendiente por Modificar
{
		int i;
		unsigned long ulPulseWidth=0;//ancho_pulso=0;
		unsigned long ulPwmAddress=0;//direccion_pwm=0;
		char          cControl;//auxiliar=0;
		while(1)
		{
			Semaphore_pend(SmphTaskWritePwm, 51.55);

			for(i=0;i<8;i++)
			{
				if(bChangeChannelPwm[i] == true)
				{
					bChangeChannelPwm[i] = false;
					ulPwmAddress = i;
				    cControl = PrimaryDataTable[i+26].control &112;
				    cControl = cControl << 3;
				    ulPulseWidth =PrimaryDataTable[i+26].information;
				    ulPulseWidth = ulPulseWidth | cControl;
				    PWMPORT(ulPwmAddress,ulPulseWidth);
				}
			}
		}
}

//-------------------------------------------\\
//         CLOCK READ ANALOGA INPUT PORT
//-------------------------------------------\\

void ClockTaskReadAdc(void)
{
	Semaphore_post(SmphTaskReadAdc0);
	Semaphore_post(SmphTaskReadAdc1);
}
void TaskReadAdc0(void)
{
	unsigned int  pui32ADC0Value[5];
	unsigned short  auxiliar, corrimiento;

	int i;
	while(1){
	Semaphore_pend(SmphTaskReadAdc0,BIOS_WAIT_FOREVER);
    ADCHardwareOversampleConfigure(ADC0_BASE, 4); //4  muestras por cada nueva conversion
    ADCSequenceDisable(ADC0_BASE, 0);  //DESACTIVA EL SEQUENCIADOR 1 DEL CONVERSOR ANALOGICO DIGITAL

    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR,0);

    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH1);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH2);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_CH3);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ADC_CTL_CH4 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 0);


    ADCIntClear(ADC0_BASE, 0);
    ADCProcessorTrigger(ADC0_BASE, 0);

     while(!ADCIntStatus(ADC0_BASE, 0, false)) //cuando salga del bucle es que se ha completado la conversion  y ya se puede leer el adc
             {
             }
     ADCSequenceDataGet(ADC0_BASE, 0, pui32ADC0Value);

     for(i=0;i<5;i++)    // Se manejan los canales A0 , A1, A2, A3 y A4
     {
    	 valor_pas_pre[i].X2=valor_pas_pre[i].X1;
    	 valor_pas_pre[i].X1=valor_pas_pre[i].X0;
		 valor_pas_pre[i].Y2=valor_pas_pre[i].Y1;
    	 valor_pas_pre[i].Y1=valor_pas_pre[i].Y0;
		 valor_pas_pre[i].X0 = pui32ADC0Value[i];
		 //valor_pas_pre[i].Y0=((valor_pas_pre[i].X0*a0)+(valor_pas_pre[i].X1*a1)-(valor_pas_pre[i].Y1*b1) )/b0;
		 valor_pas_pre[i].Y0=((valor_pas_pre[i].X0*a0)+(valor_pas_pre[i].X1*a1) +(valor_pas_pre[i].X2*a2)-(valor_pas_pre[i].Y1*b1) - (valor_pas_pre[i].Y2*b2) )/b0;
		 auxiliar = valor_pas_pre[i].Y0-1;
		 auxiliar = auxiliar & 4092;
		 auxiliar = auxiliar >> 2;
		 if(auxiliar>1023){
			 auxiliar=1023;
		 }

		 PrimaryDataTable[i+16].address= i+16;
		 PrimaryDataTable[i+16].information = auxiliar & 127;
		 corrimiento = auxiliar & 896;
		 corrimiento = corrimiento >> 3;
		 PrimaryDataTable[i+16].control = PrimaryDataTable[i+16].control & 15;
		 PrimaryDataTable[i+16].control = PrimaryDataTable[i+16].control | corrimiento;
     }
	}
}
void TaskReadAdc1(void)
{
	unsigned int pui32ADC1Value[5];
	unsigned short auxiliar, corrimiento;
	unsigned short i,j;
   while(1)
   {
	     Semaphore_pend(SmphTaskReadAdc1,BIOS_WAIT_FOREVER);

	     ADCHardwareOversampleConfigure(ADC1_BASE, 4);
	     ADCSequenceDisable(ADC1_BASE, 0);  //DESACTIVA EL SEQUENCIADOR 1 DEL CONVERSOR ANALOGICO DIGITAL

	     ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	     ADCSequenceStepConfigure(ADC1_BASE, 0, 0, ADC_CTL_CH5);
	     ADCSequenceStepConfigure(ADC1_BASE, 0, 1, ADC_CTL_CH6);
	     ADCSequenceStepConfigure(ADC1_BASE, 0, 2, ADC_CTL_CH7);
	     ADCSequenceStepConfigure(ADC1_BASE, 0, 3, ADC_CTL_CH8);
	     ADCSequenceStepConfigure(ADC1_BASE, 0, 4, ADC_CTL_CH9 | ADC_CTL_IE | ADC_CTL_END);
	     ADCSequenceEnable(ADC1_BASE, 0);

	     ADCIntClear(ADC1_BASE, 0);
	     ADCProcessorTrigger(ADC1_BASE, 0);

	      while(!ADCIntStatus(ADC1_BASE, 0, false)) // Para que la conversion se complete cuando salga del bucle es que se ha completado la conversion  y ya se puede leer el adc
	              {
	              }

	     ADCSequenceDataGet(ADC1_BASE, 0, pui32ADC1Value);

		  for(i=0;i<5;i++)    // Se manejan los canales A5 , A6, A7, A8 y A9
		  {
			 j = i+5;

			 valor_pas_pre[j].X2=valor_pas_pre[j].X1;
			 valor_pas_pre[j].X1=valor_pas_pre[j].X0;
			 valor_pas_pre[j].Y2=valor_pas_pre[j].Y1;
			 valor_pas_pre[j].Y1=valor_pas_pre[j].Y0;

			 valor_pas_pre[j].X0 = pui32ADC1Value[i];
			 //valor_pas_pre[j].Y0=((valor_pas_pre[j].X0*a0)+(valor_pas_pre[j].X1*a1)-(valor_pas_pre[j].Y1*b1))/b0;
			 valor_pas_pre[j].Y0=((valor_pas_pre[j].X0*a0)+(valor_pas_pre[j].X1*a1) +(valor_pas_pre[j].X2*a2)-(valor_pas_pre[j].Y1*b1) - (valor_pas_pre[j].Y2*b2) )/b0 ;
			 auxiliar = valor_pas_pre[j].Y0 -1;
			 auxiliar = auxiliar & 4092; //Elimina los dos bits de menor peso
			 auxiliar = auxiliar >> 2;
			 if(auxiliar>1023){
				 auxiliar=1023;
			 }
			 PrimaryDataTable[j+16].address= j+16;
			 PrimaryDataTable[j+16].information = auxiliar & 127;
			 corrimiento = auxiliar & 896;
			 corrimiento = corrimiento >> 3;
			 PrimaryDataTable[j+16].control = PrimaryDataTable[j+16].control & 15;
			 PrimaryDataTable[j+16].control = PrimaryDataTable[j+16].control | corrimiento;
		  }
   }
}

//-------------------------------------------\\
//         METHODS CALLED FOR TASKS
//-------------------------------------------\\

void PWMPORT(unsigned long ulDire, unsigned long ulWidth )
{
	fNewPulseWidth[ulDire]= (float)ulWidth/1024.0f;
	//----------------------------------------------------
	//                       PWM0-	PWM1
	//----------------------------------------------------
		TimerMatchSet(TIMER2_BASE, TIMER_A, (ulPeriod[0] * fNewPulseWidth[0]));   //WIDTH PWM0
		TimerMatchSet(TIMER2_BASE, TIMER_B, (ulPeriod[1] * fNewPulseWidth[1]));   //WIDTH PWM1
	//----------------------------------------------------
	//                       PWM2-PWM3
	//----------------------------------------------------
	    TimerMatchSet(TIMER3_BASE, TIMER_A, (ulPeriod[2] * fNewPulseWidth[2]));   //WIDTH PWM2
	 	TimerMatchSet(TIMER3_BASE, TIMER_B, (ulPeriod[3] * fNewPulseWidth[3]));   //WIDTH PWM3
	//----------------------------------------------------
	//                       PWM4-PMW5
	//----------------------------------------------------
		 TimerMatchSet(WTIMER1_BASE, TIMER_A, (ulPeriod[4] * fNewPulseWidth[4]));   //WIDTH PWM4
		 TimerMatchSet(WTIMER1_BASE, TIMER_B, (ulPeriod[5] * fNewPulseWidth[5]));   //WIDTH PWM5
	//----------------------------------------------------
	//                       PWM6-PMW7
	//----------------------------------------------------
    	 TimerMatchSet(WTIMER4_BASE, TIMER_A, (ulPeriod[6] * fNewPulseWidth[6]));   //WIDTH PWM4
		 TimerMatchSet(WTIMER4_BASE, TIMER_B, (ulPeriod[7] * fNewPulseWidth[7]));   //WIDTH PWM5
}

/*
void EnableAdcModules(short sAddress)
{


	 if ((((sAddress>=1)&&(sAddress<=4))||((sAddress>=9)&&(sAddress<=10)))&&(bInitGpioE==true))
	 		 		  {
	 		 		  	 SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	 		 		  	 bInitGpioE=false;
	 		 		  }
	 if ((bInitModAdc0==true)||(bInitModAdc1==true))
		 {
		 	 if ((sAddress>=1)&&(sAddress<=5))
		 	 	 {
		 		 	 SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
		 		 	 bInitModAdc0=false;
		 	 	 }
		 	 else if ((sAddress>=6)&&(sAddress<=10))
		 	 	 {
		 		 	 SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
		 		 	 bInitModAdc1=false;
		 	 	 }
		 }

	switch (sAddress)
	{
		case 1:
			   GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);////---A0
		       break;

		case 2:
		 	   GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);////---A1
			   break;

		case 3:
			   GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);////---A2
			   break;

		case 4:
		       GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);////---A3
		       break;

		case 5:
		       GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_3);////---A4
		       break;

		case 6:
		 	   GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_2);////---A5
		       break;

		case 7:
			   GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_1);////---A6
		       break;

		case 8:
		       GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_0);////---A7
		       break;

		case 9:
			   GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);////---A8
			   break;

		case 10:
		 	    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_4);////---A9
		 	    break;

	}
}

*/
