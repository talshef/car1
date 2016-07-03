/*
 * sensor.c
 *
 *  Created on: May 22, 2016
 *      Author: talsheff
 */
#include "TFC.h"
#include "mcg.h"

void ServosConfig (){
	
	//servo 1-left-TPM1 channel 1
	
	PORTE_PCR21 = PORT_PCR_MUX(3);
	PORTE_PCR29 = PORT_PCR_MUX(0);
	TPM1_C1SC |= TPM_CnSC_MSB_MASK  |TPM_CnSC_ELSB_MASK;
	TPM1_C1V = ZeroDegree;//start at degree 0 -look forward
	int i;
	for(i=0;i<10000;i++);
	for(i=0;i<10000;i++);
	//servo 2-right-TPM0 channel 4
	PORTE_PCR31 = PORT_PCR_MUX(3);
	PORTE_PCR30 = PORT_PCR_MUX(0);
	TPM0_C4SC |= TPM_CnSC_MSB_MASK  |TPM_CnSC_ELSB_MASK;
	TPM0_C4V = FullDegree;//start at degree 0 -look forward
	
	for(i=0;i<1000000;i++);
	for(i=0;i<100000;i++);
}


void DistSensConfig (){
	PORTD_PCR5 = PORT_PCR_MUX(0);
	PORTD_PCR6 = PORT_PCR_MUX(0);
	
	
}


void UltaSoincConfig(){

	enable_irq(INT_PORTD-16); // Enable Interrupts 
	set_irq_priority (INT_PORTD-16,1);  // Interrupt priority = 0 = max
	
	
	PORTD_PCR2 = PORT_PCR_MUX(1);//|PORT_PCR_PS_MASK;//| PORT_PCR_PE_MASK ;
	PORTD_PCR3 = PORT_PCR_MUX(1);
	//PORTD_PCR2 |=PORT_PCR_IRQC(0xc);
	GPIOD_PDDR &= (~PORT_LOC(2)||~PORT_LOC(3));
	
	PORTD_ISFR |= 0xc;  // clear interrupt flag bit of PTD6
	
	GPIOD_PDDR |= PORT_LOC(1);
	GPIOD_PSOR=0x2;
	
	
}


void ColorSensConfig (){
	PORTE_PCR2= PORT_PCR_MUX(1)|PORT_PCR_PS_MASK;
	PORTE_PCR3= PORT_PCR_MUX(1)|PORT_PCR_PS_MASK;
	PORTE_PCR4= PORT_PCR_MUX(1)|PORT_PCR_PS_MASK;
	PORTE_PCR5= PORT_PCR_MUX(1)|PORT_PCR_PS_MASK;
	PORTA_PCR16= PORT_PCR_MUX(1)|PORT_PCR_PS_MASK;
	GPIOE_PDDR &= ~(PORT_LOC(2)|PORT_LOC(3)|PORT_LOC(4)|PORT_LOC(5));
	GPIOA_PDDR &= ~(PORT_LOC(16));
}


void Servo1SetPos (int Servo1Position){
	
	TPM1_C1V=Servo1Position*3.58+ZeroDegree;
}

void Servo2SetPos (int Servo2Position){
	
	TPM0_C4V=FullDegree-Servo2Position*3.96;
}

void IrScan(){
	int i;
	turnOffUs();
	EnqueueString("qwer");
	EnqueueChar('i');
	EnqueueChar('\n');
	EnqueueInt16((uint16_t)x);
	EnqueueInt16((uint16_t)y);
	if(carDirection==-1)EnqueueInt16((uint16_t)(carDirection+1));
	else EnqueueInt16((uint16_t)carDirection);
	TPM1_C1V = ZeroDegree;//start at degree 0 -look forward
	delay(50);
	TPM0_C4V = FullDegree;//start at degree 180 -look forward
	delay(150);
	int counter=0;
	while (TPM0_C4V>=FullDegree-3.96*99){//99 degree scan
		ADC0_SC1A=ADC_SC1_ADCH(7);
		TPM0_C4V-=9;
		delay(25);
		IrSampels[43-counter]=ADC0_RA;
		ADC0_SC1A=ADC_SC1_ADCH(6);
		TPM1_C1V+=9;
		delay(25);
		IrSampels[counter+44]=ADC0_RA;
		counter++;
	}
	
	delay(25);
	Servo1SetPos(7);
	delay(25);
	Servo2SetPos(7);
	delay(50);

	//conversion from adc sample to distance- left servo
	for(i=0;i<44;i++){
		IrSampels[i]=DistanceMeasuring1(IrSampels[i]);
		EnqueueInt16(IrSampels[i]);
	}
	//conversion from adc sample to distance-right servo
	for(i=44;i<88;i++){
			IrSampels[i]=DistanceMeasuring2(IrSampels[i]);
			EnqueueInt16(IrSampels[i]);
		}
	//left servo
	for(i=0;i<44;i++){
		//removing noises
		if((IrSampels[i]<140)&&(i==0||(abs(IrSampels[i-1]-IrSampels[i])<15&&abs(IrSampels[i+1]-IrSampels[i])<15))){
			//calculate the coordination of the block
			int xIndex=(int)round(x-0.5-(sin(0.01745*((43-i)*2.51 + carDirection))*IrSampels[i]/5));
			int yIndex=(int)round(y+cos(0.01745*((43-i)*2.51 + carDirection))*IrSampels[i]/5);
			//adding blocks to metrix
			if((xIndex/6+1>0&&xIndex/6+1<15)&&yIndex/6>=0&&yIndex/6<16){
				if(maze[xIndex/6+1][yIndex/6]=='r') maze[xIndex/6][yIndex/6]='H';
				else	if(xIndex/6==(int)(x/6)&&xIndex+2<(int)x) maze[xIndex/6][yIndex/6]='H';
				else	maze[xIndex/6+1][yIndex/6]='H';
			}
		}
		
	}
	//right servo
	for(i=44;i<88;i++){
		//removing noises
		if((IrSampels[i]<140)&&(i==87||(abs(IrSampels[i-1]-IrSampels[i])<15&&abs(IrSampels[i+1]-IrSampels[i])<15))){
			//calculate the coordination of the block
			int xIndex=(int)round(x+0.5+(sin(0.01745*((i-44)*2.27 + carDirection))*IrSampels[i]/5));
			int yIndex=(int)round(y+cos(0.01745*((i-44)*2.27 + carDirection))*IrSampels[i]/5);
			//adding blocks to metrix
			if((xIndex/6+1>0&&xIndex/6+1<15)&&yIndex/6>=0&&yIndex/6<16){
				if(maze[xIndex/6+1][yIndex/6]=='r') maze[xIndex/6+2][yIndex/6]='H';
				else if(xIndex/6==(int)x/6&&xIndex-2>(int)x) maze[xIndex/6+2][yIndex/6]='H';
				else maze[xIndex/6+1][yIndex/6]='H';
			}
			
		}
	}
	
	EnqueueChar(0x1A);
	EnqueueChar('\n');
	
	
	EnqueueChar(0xff);
	UART0_C2 |=  UART_C2_TIE_MASK;  //enable transmit interrupt for start sendding
	turnOnUs();
}	
void IrScanEnd(){
	int i;
	
	

	TPM1_C1V = ZeroDegree;//start at degree 0 -look forward
	
	delay(250);
	TPM0_C4V = FullDegree;//start at degree 180 -look forward
	delay(50);
	
	int counter=0;
	while (TPM0_C4V>=FullDegree-3.96*80){//80 degree scan
			
		ADC0_SC1A=ADC_SC1_ADCH(7);
		
		TPM0_C4V-=9;
		delay(25);
		
		IrSampels[43-counter]=DistanceMeasuring1(ADC0_RA);
		
		ADC0_SC1A=ADC_SC1_ADCH(6);
		TPM1_C1V+=9;
		delay(25);
		
		IrSampels[counter+44]=DistanceMeasuring2(ADC0_RA);
		counter++;
		
		
	}
	delay(25);
	TPM1_C1V = ZeroDegree;//start at degree 0 -look forward
	
	delay(25);
	TPM0_C4V = FullDegree;//start at degree 180 -look forward
	delay(50);

	
	

	
	
}	



//******************fill the distance array linearly*********************//
void DistanceMeasure (){
	int i;
	int stepleft=(Irdistance[0][5]-Irdistance[0][0])/5;
	int stepright=(Irdistance[1][5]-Irdistance[1][0])/5;
	for(i=1;i<5;i++){
		Irdistance[0][i]=Irdistance[0][0]+stepleft*i;
		Irdistance[1][i]=Irdistance[1][0]+stepright*i;
	}
	int j;
	for(j=0;j<13;j++){
		stepleft=(Irdistance[0][j*10+15]-Irdistance[0][j*10+5])/10;
		stepright=(Irdistance[1][j*10+15]-Irdistance[1][j*10+5])/10;
		for(i=1;i<10;i++){
			Irdistance[0][j*10+5+i]=Irdistance[0][j*10+5]+stepleft*i;
			Irdistance[1][j*10+5+i]=Irdistance[1][j*10+5]+stepright*i;
		}
	}
	
	
	
}

//*******************enter default values to IR distance******************//
void defultDistance(){
	Irdistance[0][0]=47279;
	Irdistance[0][5]=43832;
	Irdistance[0][15]=35163;
	Irdistance[0][25]=27632;
	Irdistance[0][35]=23340;
	Irdistance[0][45]=21354;
	Irdistance[0][55]=17369;
	Irdistance[0][65]=15520;
	Irdistance[0][75]=13985;
	Irdistance[0][85]=12598;
	Irdistance[0][95]=11868;
	Irdistance[0][105]=11100;
	Irdistance[0][115]=10344;
	Irdistance[0][125]=9645;
	Irdistance[0][135]=9187;
	
	
	
	Irdistance[1][0]=47293;
	Irdistance[1][5]=43218;
	Irdistance[1][15]=34387;
	Irdistance[1][25]=26817;
	Irdistance[1][35]=22440;
	Irdistance[1][45]=20391;
	Irdistance[1][55]=16427;
	Irdistance[1][65]=14520;
	Irdistance[1][75]=13386;
	Irdistance[1][85]=12422;
	Irdistance[1][95]=11220;
	Irdistance[1][105]=10170;
	Irdistance[1][115]=9860;
	Irdistance[1][125]=9673;
	Irdistance[1][135]=8632;
	DistanceMeasure ();
}


//******************find the distance for sample**************************//
int DistanceMeasuringSearch1 (int DisSamp1,int start,int end){
	int index=(start+end)/2;
	if(index==0||index==135) return index+15;
	if(Irdistance[0][index]<DisSamp1){
		if(Irdistance[0][index-1]<DisSamp1)return DistanceMeasuringSearch1 (DisSamp1,start,index-1);
		return index+14;
	}
	else if(Irdistance[0][index]>DisSamp1){
			if(Irdistance[0][index+1]>DisSamp1)return DistanceMeasuringSearch1 (DisSamp1,index+1,end);
			return index+16;	
	}else return index+15;
		
}	

int DistanceMeasuring1 (int DisSamp1){
	return DistanceMeasuringSearch1 (DisSamp1,0,135);
	 
}

int DistanceMeasuringSearch2 (int DisSamp2,int start,int end){
	int index=(start+end)/2;
	if(index==0||index==135) return index+15;
	if(start==end) return index+15;
	if(Irdistance[1][index]<DisSamp2){
		if(Irdistance[1][index-1]<DisSamp2)return DistanceMeasuringSearch2 (DisSamp2,start,index-1);
		return index+14;
	}
	else if(Irdistance[1][index]>DisSamp2){
			if(Irdistance[1][index+1]>DisSamp2)return DistanceMeasuringSearch2 (DisSamp2,index+1,end);
			return index+16;	
	}else return index+15;
		
}	

int DistanceMeasuring2 (int DisSamp2){
	return DistanceMeasuringSearch2 (DisSamp2,0,135);
	 
}



//******************turn off/on UltraSonic**************************//
turnOffUs(){

	PORTD_PCR2 &=~PORT_PCR_IRQC(0xF);
	PORTD_PCR3 &=~PORT_PCR_IRQC(0xF);
	PIT_TCTRL0=0;
}
turnOnUs(){
	PORTD_PCR2 |=PORT_PCR_IRQC(0xc);
	PORTD_PCR3 |=PORT_PCR_IRQC(0xc);
	PIT_TCTRL0 = PIT_TCTRL_TEN_MASK | PIT_TCTRL_TIE_MASK; 
}

//******************turn off/on Ir**************************//
turnOffIr(){
	
	ADC0_SC1A = ADC_SC1_ADCH(7);
}
turnOnIr(){
	
	ADC0_SC1A = ADC_SC1_ADCH(7)|ADC_SC1_AIEN_MASK;
	adcMux=0;
}
