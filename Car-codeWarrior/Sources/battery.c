# include "TFC.h"

void battLedConfig(){
	PORTE_PCR22 = PORT_PCR_MUX(0);  
	PORTB_PCR8 = PORT_PCR_MUX(1);
	PORTB_PCR9 = PORT_PCR_MUX(1);
	PORTB_PCR10 = PORT_PCR_MUX(1);
	GPIOB_PDDR |= PORT_LOC(8)|PORT_LOC(9)|PORT_LOC(10);  // PTB8/9/10 is Output
}

void BattLedState (int BattSamp){
	int bat=6.05+(ADC0_RA*3.3/0xFFFF-1.02)*6.905;
	int led=(int)((BattSamp*3.3/0xFFFF-1.02)/0.02625)*0x100;
	GPIOB_PCOR=0x700;
	GPIOB_PSOR=led;
	
}

void BattSample(){
	ADC0_SC1A=ADC_SC1_ADCH(3); //Checks battery level
	delay(50);
	battSample=(ADC0_RA*3.3/0xFFFF-1.02)*100;
}
