/*
 * main implementation: use this 'C' sample to create your own application
 *
 */

#include<stdio.h>
#include<stdlib.h>
//#include "derivative.h" /* include peripheral declarations */
# include "TFC.h"



//#define MAX_SPEED	32
static unsigned int testNum=0;

int ServoDir=1; //1  open Servos  ,-1 close Servos
  //				3-calibrate ir sensor
int caliCounter=-1;

void EnqueueFloat(float x);
void EnqueueChar(char j);
char Dequeue();
void EnqueueString(char* str);
float* EncoderSensing();
void Servo1SetPos (int Servo1Position);
void Servo2SetPos (int Servo2Position);
void BattLedState (int BattSamp);
void DistanceMeasure();
int DistanceMeasuringSearch1 (int DisSamp1,int start,int end);
int DistanceMeasuring1 (int DisSamp1);
void startMotors();


/* Variable for test*/
int leftIrTemp=0;
int rightIrTemp=0;
double speedLeftTemp=0;
double speedRightTemp=0;
double MAX_SPEED=40;
double sonicRightDistanceTemp=0;
double sonicLeftDistanceTemp=0;

double driveSpeedLeft=40;
double driveSpeedRight=40.2;
int stopFlag=0;





int main(void){
	ClockSetup();
	InitGPIO();
	InitUARTs();
	TPMSetup();
	EncoderConfig();
	DistSensConfig ();
	adc_init();
	initRf();
	dma_init();
	dtf_init();
	battLedConfig();
	MotorConfig();
	InitPIT();
	defultDistance();
	ColorSensConfig ();
	ServosConfig ();
	UltaSoincConfig();
	initMaze();
	BattSample();
	
	//send char to clean PC buffer
	EnqueueString("qwer");
	EnqueueChar(0x1A);
	EnqueueChar('\n');
	UART0_C2 |=  UART_C2_TIE_MASK;	
	
	
	delay(2000);
	//send to PC that the car is ready to start
	EnqueueChar(0x1A);
	EnqueueChar('\n');
	EnqueueChar(0xff);
	UART0_C2 |=  UART_C2_TIE_MASK;	
	EnqueueString("qwer");
	EnqueueChar('o');
	EnqueueChar('\n');
	EnqueueChar(0x1A);
	EnqueueChar('\n');
	EnqueueChar(0xff);
	UART0_C2 |=  UART_C2_TIE_MASK;	
	RGB_LED_ON;
	delay(1000);
	RGB_LED_OFF;
	RED_LED_ON;
	delay(1000);
	RGB_LED_OFF;
	BLUE_LED_ON;
	delay(1000);
	RGB_LED_OFF;
	GREEN_LED_ON;
	delay(1000);
	RGB_LED_OFF;
	RED_LED_ON;
	BLUE_LED_ON;
	delay(1000);
	RGB_LED_OFF;
	InstructIndex=0;
	
	
	RGB_LED_OFF;
	
	int distance=35;

	state=100;
	findHole();
	
	while(1){
		if(state==9){
			ADC0_SC1A=ADC_SC1_ADCH(3); //Checks battery level
			delay(25);
			BattLedState(ADC0_RA);
			//delay(250);
			
			if(drivingInstructions[InstructIndex].opp=='e'){ //end of instructions
				state=30;
				InstructIndex=0;
				RGB_LED_OFF;
				RED_LED_ON;
				BLUE_LED_ON;
				if(carDirection>5||carDirection<-5) turnPivot(0);
				findEnd();
				
			}
			else if(drivingInstructions[InstructIndex].opp=='d'){ // drive straight not knowing of obstacles
				strightTicks=drivingInstructions[InstructIndex].num;

					startMotors(9375);
					adcMux=0;
					turnOnUs();
					state=0;
					InstructIndex++;
					
			}
			else if(drivingInstructions[InstructIndex].opp=='h'){  //drive straight knowing obstacle in front
				strightTicks=drivingInstructions[InstructIndex].num;
				
					startMotors(9375);
					state=1;
					InstructIndex++;
					turnOnUs();
			
			}
			else  if(drivingInstructions[InstructIndex].opp=='t'){ //make a turn
				turnPivot((int)(drivingInstructions[InstructIndex].num));
				InstructIndex++;
			}
			
			else  if(drivingInstructions[InstructIndex].opp=='o'){ //find path
				strightTicks=drivingInstructions[InstructIndex].num;
				adcMux=0;
				startMotors(9375);
				state=6;
				InstructIndex++;
			}
				
		}
		
	
		/************ going straight by ticks-Ir Long Distance 'd'**********************/
		while(state==0){
			adcMux=0;
			delay(150);
			
			TicksFix();
			delay(100);
			ADC0_SC1A=ADC_SC1_ADCH(7) |ADC_SC1_AIEN_MASK;
			delay(25);
			ADC0_SC1A=ADC_SC1_ADCH(6) |ADC_SC1_AIEN_MASK;
			delay(25);
			if((leftIr>Irdistance[0][35]||rightIr>Irdistance[1][35])||(sonicRightDistance<=24||sonicLeftDistance<=24)){
				stopMotor();
				if(carDirection>5||carDirection<-5) turnPivot(0);
				 state=4;
			}
			if(totalEncoder>6000){
				strightTicks -=totalEncoder;
				stopMotor();
				if(carDirection>5||carDirection<-5) turnPivot(0);
				startMotors(9375);
			}
			
			if((y+totalEncoder*cos(carDirection*0.01745)/21.6/5)>=90){
				x+=(totalEncoder*sin(carDirection*0.01745)/21.6)/5;
				y+=(totalEncoder*cos(carDirection*0.01745)/21.6)/5;
				totalEncoder=0;
				totalEncoderLeft=0;
				strightTicks=4000;//Continue 60 cm
				state=7;
				RGB_LED_OFF;	
				GREEN_LED_ON;
				BLUE_LED_ON;
			}
		
		
		}
		/**********final state*****///////
		while(state==7){
			delay(150);
			TicksFix();
			delay(100);
			adcMux=0;
			ADC0_SC1A=ADC_SC1_ADCH(7) |ADC_SC1_AIEN_MASK;
			delay(25);
			ADC0_SC1A=ADC_SC1_ADCH(6) |ADC_SC1_AIEN_MASK;
			delay(25);
			if((leftIr>Irdistance[0][40]||rightIr>Irdistance[1][40])){
				RGB_LED_OFF;
				RED_LED_ON;
				stopMotor();
				if(carDirection>5||carDirection<-5) turnPivot(0);
				if(x>=36&&x<=47) {				
					findEnd();					
				}
				else{
					initMaze();
					findPath();
					
				
					if(x<41){
						drivingInstructions[0].opp='t';
						drivingInstructions[0].num=90;
						drivingInstructions[1].opp='h';
						drivingInstructions[1].num=(41.5-x)*108;
						drivingInstructions[2].opp='t';
						drivingInstructions[2].num=0;
						drivingInstructions[3].opp='e';
						drivingInstructions[3].num=0;
					}else{
						drivingInstructions[0].opp='t';
						drivingInstructions[0].num=-90;
						drivingInstructions[1].opp='h';
						drivingInstructions[1].num=(x-41.5)*108;
						drivingInstructions[2].opp='t';
						drivingInstructions[2].num=0;
						drivingInstructions[3].opp='e';
						drivingInstructions[3].num=0;
					}
					RGB_LED_ON;
					InstructIndex=0;
					state=9;
				}
			}
				
			if (totalEncoder>strightTicks){ //end of drive
				GPIOC_PCOR=0X0000400;
				GPIOC_PSOR=0X0000040;
				TPM2_C1V = 9375;
				TPM1_C0V = 9375-900;
				delay(2000);
				GPIOC_PSOR=0X0000400;
				delay(250);
				GPIOC_PCOR=0X0000040;
				GPIOC_PSOR=0X0000400;
				delay(2000);
				stopMotor();
				state=30;
			}
				
		}
		/**************going straight by ticks- UltraSonic Short Distance   'h'************************/
		while(state==1){
			delay(150);
			TicksFix();
			delay(150);
			if ((totalEncoder>strightTicks)||(sonicRightDistance<=24||sonicLeftDistance<=24)){ //end of drive
				stopMotor();
				
				state=9;
			}
			
		
			
		}
		
		/**************Reverse************************/
		while(state==2){
			delay(200);
			if (totalEncoder>strightTicks){ //end of drive
				stopMotorReverse();
				delay(500);
				state=4;
			}
		}
					
	/************ find Path**********************/		
		if(state==4){
			int left;
			int right;
			InstructIndex=0;
			initMaze();
			ADC0_SC1A=ADC_SC1_ADCH(7);
			delay(25);
			left=DistanceMeasuring1(ADC0_RA);
			ADC0_SC1A=ADC_SC1_ADCH(6);
			delay(25);
			right=DistanceMeasuring2(ADC0_RA);
			if(left<85||right<85)
				IrScan();
			//sendMaze();
			findPath();
		}
	
	/************ going in the end **********************/
		while(state==5){
			delay(150);
			TicksFix();
			delay(150);
			if (totalEncoder>strightTicks){ //end of drive
				GPIOC_PCOR=0X0000400;
				GPIOC_PSOR=0X0000040;
				TPM2_C1V = 9375;
				TPM1_C0V = 9375-900;
				delay(2000);
				GPIOC_PSOR=0X0000400;
				delay(250);
				GPIOC_PCOR=0X0000040;
				GPIOC_PSOR=0X0000400;
				delay(2000);
				stopMotor();
				state=30;
			}
			
			
		}
		
		//**************************** out of the circle****************************//
		while(state==6){
			delay(150);
			TicksFix();
			delay(150);
			if (totalEncoder>strightTicks){ //end of drive
				int tempX=x+(totalEncoder*sin(carDirection*0.01745)/21.6)/5;
				if(abs(tempX-((int)tempX/6)*6)>1&&abs(tempX-((int)tempX/6)*6)<4){
					stopMotor();
					
					turnPivot(0);
					int left;
					int right;
					InstructIndex=0;
					initMaze();
					ADC0_SC1A=ADC_SC1_ADCH(7);
					delay(25);
					left=DistanceMeasuring1(ADC0_RA);
					ADC0_SC1A=ADC_SC1_ADCH(6);
					delay(25);
					right=DistanceMeasuring2(ADC0_RA);
					if(left<85||right<85)
						IrScan();
					
					findPath();
					turnOnUs();
					GREEN_LED_ON;
					InstructIndex=0;
				}
				else{
					RED_LED_ON;
					strightTicks+=50;
				}
				
			}
			
				
					
				}
				
	}
	
	return 0;
}


//-----------------------------------------------------------------
//  PORTD - ISR = Interrupt Service Routine
//-----------------------------------------------------------------
void PORTD_IRQHandler(void){
	pitVal=PIT_CVAL0;
	volatile unsigned int i;
	
	
	if (PORTD_ISFR & 0x4) {//ptd2- right sonic input 
	 	sonicRightDistance=(sonicCapture-pitVal)/1392-12;
	 	PORTD_ISFR |= 0x4;  // clear interrupt flag bit of PTD2	
	}
	if(PORTD_ISFR & 0x8){//ptd3- left sonic input
		sonicLeftDistance=(sonicCapture-pitVal)/1392-12;
		PORTD_ISFR |= 0x8;  // clear interrupt flag bit of PTD2	
	}
	if (PORTD_ISFR & SW_POS) {  //DTMF-ptd7
		int num=0;	
		char str[20];
		if(GPIOC_PDIR&0x0800) num=1;
		if(GPIOC_PDIR&0x1000) num=num|0x02;
		if(GPIOC_PDIR&0x2000)  num=num|0x04;
		if(GPIOA_PDIR&0x20000)  num=num|0x08;
		switch (num){
			case 1:
				
			break;
			case 2:
				PIT_TCTRL0 = PIT_TCTRL_TEN_MASK | PIT_TCTRL_TIE_MASK; 
				startMotors(9375);
				strightTicks=5000;
				state=1;
			break;
			case 3:
				
				findHole();
			break;
			case 4:
				state=4;
			break;
			case 5:
				sendMaze();
			break;
			case 6:
				 
					
					 EnqueueChar('p');
					 EnqueueChar('\n');
					 int m,n;
					 for(m=18;m>=0;m--){
					 	for (n=1; n<15;n++){
					 		EnqueueChar(maze[n][m]);
					 	}
					 }
						
					 EnqueueChar(0x1A);
					 EnqueueChar('\n');	
					 UART0_C2 |=  UART_C2_TIE_MASK;  //enable transmit interrupt for start sendding
					
			break;
			case 7:
			
					num=0;		
			break;
			
					
		}
		PORTD_ISFR |= 0x00000080;  // clear interrupt flag bit of PTD7		
			
	}
}





//-----------------------------------------------------------------
//  TPM0 - ISR
//-----------------------------------------------------------------
void FTM0_IRQHandler(void){				//ptm

		
		if(TPM0_STATUS&0x4){ //capture right encoder
			totalEncoder++;
			encouderCounterRight++;
			TPM0_C2SC|= TPM_CnSC_CHF_MASK;
		}
		if(TPM0_STATUS&0x8){ //capture left encoder
			totalEncoderLeft++;
			encouderCounterLeft++;
			TPM0_C3SC|= TPM_CnSC_CHF_MASK;
		}
}



void PIT_IRQHandler(){
	//used for sand that ultrasonic wave
	int i;
	PIT_TFLG0 = 1; //clear the Pit 0 Irq flag
	GPIOD_PSOR=0x2;
	for(i=0;i<10;i++);
	GPIOD_PCOR=0x2;
	sonicCapture=PIT_CVAL0 ;
}






