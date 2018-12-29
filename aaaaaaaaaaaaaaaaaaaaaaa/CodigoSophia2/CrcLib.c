//*****************************************************************************
//
// led_task.c - A simple flashing LED task.
//
// Copyright (c) 2012 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 9453 of the EK-LM4F120XL Firmware Package.
//
//*****************************************************************************


#include <stdint.h>
#include <stdbool.h>







char CRCTrama(char cData0, char cData1, char cData2)
{
	char cChar_Bin[32], cNEWCRC[7],cCRC=0,cCRC_Data=0;
	int i=0, iAux=1;

//////IGUALACION A CERO DE TODAS LAS POCISIONES DEL VECTOR
	for(i=0;i<32;i++)
	   {

		cChar_Bin[i]=0;

		if(i<7)
	      {
	    	cNEWCRC[i]=0;
	      }

	   }

//////---------->START CHAR TO BINARIO<----------//////
	for(i=0;i<32;i++)
	   {

		if(i<8)////INFORMACION
		  {
		   cChar_Bin[i]= cData0 % 2;
		   cData0=cData0/2;
		  }

		if((i>=8)&&(i<16))//// CONTROL
		  {
		   cChar_Bin[i]= cData1 % 2;
		   cData1=cData1/2;
		  }

		if((i>=16)&&(i<24))///DIRECCION
		  {
				   cChar_Bin[i]= cData2 % 2;
				   cData2=cData2/2;
		  }
	  }
//////---------->END CHAR TO BINARIO<----------//////

//////---------->STAR  CALCULO DEL CRC<----------//////
	cNEWCRC[0] = cChar_Bin[28] ^ cChar_Bin[21] ^ cChar_Bin[14] ^ cChar_Bin[7] ^  cChar_Bin[0] ^ cCRC;
	cNEWCRC[1] = cChar_Bin[29] ^ cChar_Bin[22] ^ cChar_Bin[15] ^ cChar_Bin[8] ^  cChar_Bin[1] ^ cCRC;
	cNEWCRC[2] = cChar_Bin[30] ^ cChar_Bin[23] ^ cChar_Bin[16] ^ cChar_Bin[9] ^  cChar_Bin[2] ^ cCRC;
	cNEWCRC[3] = cChar_Bin[31] ^ cChar_Bin[24] ^ cChar_Bin[17] ^ cChar_Bin[10] ^ cChar_Bin[3] ^ cCRC;
	cNEWCRC[4] = cChar_Bin[25] ^ cChar_Bin[18] ^ cChar_Bin[11] ^ cChar_Bin[4] ^ cCRC;
	cNEWCRC[5] = cChar_Bin[26] ^ cChar_Bin[19] ^ cChar_Bin[12] ^ cChar_Bin[5] ^ cCRC;
	cNEWCRC[6] = cChar_Bin[27] ^ cChar_Bin[20] ^ cChar_Bin[13] ^ cChar_Bin[6] ^ cCRC;
//////---------->END   CALCULO DEL CRC<----------//////

//////---------->STAR  BINARIO TO CHAR<----------//////
	 for(i=0;i<7;i++)
	    {
		 cCRC_Data= cCRC_Data + (cNEWCRC[i] * iAux);
	     iAux=iAux*2;
	    }
//////---------->STAR  BINARIO TO CHAR<----------//////

return cCRC_Data;
}
