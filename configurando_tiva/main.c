#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#define estado 2
int  led=8;
float var;
int main(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);  // se define el reloj, la velocidad con la qeu se va a trabajar
																					  // averiguar sobre esto...

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);  // averiguar tambien xdd
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);  // se estan habilitando del puerto F los pines 1 2 y 3


while(1)
{
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1| GPIO_PIN_2| GPIO_PIN_3, estado);
	//SysCtlDelay(1000000);
	var= SysCtlClockGet();

	SysCtlDelay(16000000);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
	//SysCtlDelay(1000000);
	SysCtlDelay(16000000);

	//if(led==8) {led=2;} else {led=led*2;}
}
}
