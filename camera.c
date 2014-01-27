#include "extern_globals.h"
#include "delay.h"
#include "camera.h"

/* revoir l'ordre des etapes !!
void Aquisition_Camera_Interrupt(void)
{
	if (i_camera_interrupt == 0)  
	{
		SIU.PGPDO[1].R &= ~0x0000000C;          // mets les 2 pattes (Clk et Start) � 0 All port line low 
   		SIU.PGPDO[1].R |= 0x00000008;           // mets start read � 1 Sensor read start High 
	}
	else if (i_camera_interrupt & 0x000001 == 1)
	{
		 SIU.PGPDO[1].R |= 0x00000004;            // clk � 1 Clock High 
	}
	else if (i_camera_interrupt == 2)
	{
   	    SIU.PGPDO[1].R &= ~0x00000008;          // start � 0
    	SIU.PGPDO[1].R &= ~0x00000004;          // clk � 0
	}
	else if (i_camera_interrupt < 260)  // 260 = 4 (i-start) + 128*2
	{
		SIU.PGPDO[1].R |= 0x00000004;   // clk � 1
        ADC.MCR.B.NSTART=1;             // launch adc conversion  Trigger normal conversions for ADC0 
	}
	else 
	{
		//desactiver ces interruptions
	}
	
	if (i_camera_inerrupt == 260) i_camera_interrupt = 0;
	else i_camera_interrupt ++;
	
}

void ADC_Interupt(void)
{
	static uint16_t valeurs_camera[128];
	valeurs_camera[i_camera_interupt/2] = ADC.CDR[14].B.CDATA; // range la sortie de l'ADC dans 
}

*/


void Acquisition_Camera(void)
{
    uint8_t i;
    uint32_t adcdata1;
    uint32_t adcdata2;
    /* Code original */
    //SIU.PGPDO[0].R &= ~0x00000014;          /* All port line low */
    //SIU.PGPDO[0].R |= 0x00000010;           /* Sensor read start High */
    //delay(250);
    //SIU.PGPDO[0].R |= 0x00000004;           /* Sensor Clock High */
    //delay(250);
    //SIU.PGPDO[0].R &= ~0x00000010;          /* Sensor read start Low */ 
    //delay(250);
    //SIU.PGPDO[0].R &= ~0x00000004;          /* Sensor Clock Low */
    //delay(250);
    
    /* En passant directement sur la carte m�re */
    SIU.PGPDO[1].R &= ~0x0000280C;          /* All port line low */
    SIU.PGPDO[1].R |= 0x00002008;           /* Sensor read start High */
    delay(250);
    SIU.PGPDO[1].R |= 0x00000804;           /* Sensor Clock High */
    delay(250);
    SIU.PGPDO[1].R &= ~0x00002008;          /* Sensor read start Low */ 
    delay(250);
    SIU.PGPDO[1].R &= ~0x00000804;          /* Sensor Clock Low */
    delay(250);
    for (i=0;i<128;i++)
    {
        delay(250);
        SIU.PGPDO[1].R |= 0x00000804;   /* Sensor Clock High */
        ADC.MCR.B.NSTART=1;             /* Trigger normal conversions for ADC0 */
        while (ADC.MCR.B.NSTART == 1) {};
        //adcdata = ADC.CDR[0].B.CDATA; // En passant par la carte de puissance...
        adcdata1 = ADC.CDR[14].B.CDATA; // Mettre la sortie de la camera sur PD[10]
        adcdata2 = ADC.CDR[4].B.CDATA; // Mettre la sortie de la camera sur PD[0]
         
        delay(250);
        SIU.PGPDO[1].R &= ~0x00000804;  /* Sensor Clock Low */
        camera_valeurs_brutes1[i] = (uint16_t)adcdata1;
        camera_valeurs_brutes2[i] = (uint16_t)adcdata2;
    }
}
