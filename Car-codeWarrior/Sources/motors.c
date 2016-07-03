

#include "TFC.h"
#include "mcg.h"
#define KP 70
// motor - Initialization
//-----------------------------------------------------------------
void MotorConfig(){
	//GPIO Configuration - output motor direction
	//motor 1
	PORTE_PCR23 = PORT_PCR_MUX(3);
	PORTC_PCR6 = PORT_PCR_MUX(1);
	PORTC_PCR5 = PORT_PCR_MUX(1);
	GPIOC_PDDR |= 0x00000060;  //OUTPUT
	
	//tpm clock 24000000 
	
	TPM2_C1SC |= TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
	TPM2_C1V = 2500;//MUDULO_TPM;
	
	
	
			
			
	//motor2
	
	PORTE_PCR20 = PORT_PCR_MUX(3);
	PORTC_PCR7 = PORT_PCR_MUX(1);
	PORTC_PCR10 = PORT_PCR_MUX(1);
	GPIOC_PDDR |= 0x00000480;  //OUTPUT
	
	
	//tpm clock 24000000 
	
	TPM1_C0SC |= TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
	TPM1_C0V = 2250;//TOP_SPEED_TPM_LEFT-800;//MUDULO_TPM;
	
	GPIOC_PCOR=0X000004E0;	//stop the motors
}

void EncoderConfig(){
	
	//encoder 1
	PORTC_PCR3 = PORT_PCR_MUX(4);
	TPM0_C2SC |=  TPM_CnSC_ELSA_MASK + TPM_CnSC_CHIE_MASK;
	TPM0_C2V = 0xFFFF; 
	
	
	
	//encoder 2
	PORTC_PCR4 = PORT_PCR_MUX(4);
	TPM0_C3SC |=  TPM_CnSC_ELSA_MASK + TPM_CnSC_CHIE_MASK;
	TPM0_C3V = 0xFFFF; 
	
	
	enable_irq(INT_TPM0-16); // Enable Interrupts 
	set_irq_priority (INT_TPM0-16,0);  // Interrupt priority = 0 = max
}

//******************turn the car**************************//
void turnPivot(int direction){
	turnOffIr();
	turnOffUs();
	int leftFinish=0;
	int rightFinish=0;
	totalEncoderLeft=0;
	totalEncoder=0;
	encouderCounterLeft=0;
	encouderCounterRight=0;
	int directionCount=0;
	if(abs((int)(direction-carDirection))<30){  //small turns
		if(direction-carDirection<0){  //left turn
			//calculate the num of encoders ticks 
			directionCount=(carDirection-direction)*32/9;
			//give Initial velocity
			speedset(1,300);
			speedset(0,300);
			GPIOC_PSOR=0X000000C0;
			delay(700);
			speedset(1,1200);
			speedset(0,1350);
			
			
		}else{    //right turn
			//calculate the num of encoders ticks 
			directionCount=(direction-carDirection)*32/9;
			//give Initial velocity
			speedset(1,300);
			speedset(0,300);
			GPIOC_PSOR=0X00000420;
			delay(700);
			speedset(1,1200);
			speedset(0,1300);
			
		}
	}
	else {  //large turn
		if(direction-carDirection<0){  //turn left
			//calculate the num of encoders ticks 
			directionCount=(carDirection-direction)*32/9;
			//give Initial velocity
			speedset(1,300);
			speedset(0,300);
			GPIOC_PSOR=0X000000C0;
			delay(500);
		
			speedset(1,1700);
			speedset(0,1700);
		}else{     //turn right
			//calculate the num of encoders ticks 
			directionCount=(direction-carDirection)*32/9;
			//give Initial velocity
			speedset(1,300);
			speedset(0,300);
			GPIOC_PSOR=0X00000420;
			delay(500);
			speedset(1,1600);
			speedset(0,1700);
		}						
			
	}
	carDirection=direction;

	int turnDone=0;
	int rightSlow=0;
	int leftSlow=0;
	//while turning		
	while(!turnDone){
		//reducing the speed 100 ticks to the end
		if(!rightSlow&&(directionCount-encouderCounterRight<100)){
			rightSlow=1;
			speedset(0,0);
			delay(80);
			speedset(0,1400);
		}
		//reducing the speed 100 ticks to the end
		if(!leftSlow&&(directionCount-encouderCounterRight<100)){
			leftSlow=1;
			speedset(1,0);
			delay(80);
			speedset(1,1300);
		}
		//right motor finish the encoder ticks needed
		if(encouderCounterRight>directionCount){
			speedset(0,100);
			GPIOC_PCOR=0X00000060;
			GPIOC_PSOR=0X00000040;
			rightFinish=1;
	
		}
		//left motor finish the encoder ticks needed
		if(encouderCounterLeft>directionCount){
					speedset(1,100);
					GPIOC_PCOR=0X00000480;
					GPIOC_PSOR=0X00000400;
					leftFinish=1;
		}			
		//finish the turn- stop the car
		if(leftFinish&&rightFinish){	
			delay(200);
			GPIOC_PCOR=0X000004E0;
			delay(300);
			int degree;
			//updating the real angle of the car 
			degree=encouderCounterLeft+encouderCounterRight-2*directionCount;
			carDirection+=leftturn*degree/3.55;
			encouderCounterLeft=0;
			encouderCounterRight=0;
			totalEncoder=0;
			totalEncoderLeft=0;
			turnDone=1;
		}
	}
	if(abs((int)(direction-carDirection)>10)) return turnPivot(direction);
}

//******************set speed to each motor**************************//
void speedset(int motor,int speed){
	if(motor==0) TPM2_C1V=speed;
	else TPM1_C0V=speed;
}


//******************start/stop the car**************************//
void startMotors(int speed){
	totalEncoder=0;
	totalEncoderLeft=0;
	encouderCounterLeft=0;
	encouderCounterRight=0;
	
	speedset(1,300);
	speedset(0,300);
	GPIOC_PSOR=0X00000440;
	delay(800);
	TPM2_C1V = 6500;		
	TPM1_C0V = 6000;
	

}

void startMotorsReverse(int speed){
	int i;
	GPIOC_PCOR=0X000004E0;
	speedset(1,300);
	speedset(0,300);
	GPIOC_PSOR=0X000000A0;
			
	delay(500);
		
	speedset(1,speed-375);
	speedset(0,speed);

}


void stopMotor(){
	 	 int i;
	 	speedset(1,300);
	 	speedset(0,300);	
	 	delay(500);
	 	GPIOC_PCOR=0X000004E0;
	 	delay(100);
	 	
		
		 x+=(totalEncoder*sin(carDirection*0.01745)/21.6)/5;
		 y+=(totalEncoder*cos(carDirection*0.01745)/21.6)/5;
		 if(totalEncoder>totalEncoderLeft-20){
		 			speedset(1,1200);
		 			delay(40);
		 			GPIOC_PSOR=0X00000400;
		 			while(totalEncoder>totalEncoderLeft);
		 			speedset(1,300);
		 			GPIOC_PCOR=0X00000400;
		 		}
		 else if(totalEncoder+10<totalEncoderLeft){
		 			speedset(0,1000);
		 			
		 			delay(40);
		 			GPIOC_PSOR=0X00000040;
		 			while(totalEncoder+10<totalEncoderLeft);
		 			speedset(0,300);
		 			GPIOC_PCOR=0X00000040;
		 		}
		 delay(100);
		GPIOC_PCOR=0X000004E0;
	
		
	
		
		
		//double degree=totalEncoderLeft-totalEncoder;
		//carDirection+=degree/3.55;
		totalEncoder=0;
		totalEncoderLeft=0;
		
		EnqueueString("qwer");
		EnqueueChar('l');
		EnqueueChar('\n');
		EnqueueInt16((uint16_t)x);
		EnqueueInt16((uint16_t)y);
		if(carDirection==-1)EnqueueInt16((uint16_t)(carDirection+1));
		else EnqueueInt16((uint16_t)carDirection);
		EnqueueChar(0x1A);
		EnqueueChar('\n');
		EnqueueChar(0xff);
		UART0_C2 |=  UART_C2_TIE_MASK;  //enable transmit interrupt for start sendding
		
		
}
 
 void stopMotorReverse(){
	 	 int i;
	 	 
	 	 speedset(1,300);
		 speedset(0,300);	
		 delay(500);
		 

		GPIOC_PCOR=0X000004E0;
		GPIOC_PSOR=0X00000440;
		delay(100);
		 speedset(1,0);
		 speedset(0,0);
		x-=(totalEncoder*sin(carDirection*0.01745)/21.6)/5;
		y-=(totalEncoder*cos(carDirection*0.01745)/21.6)/5;
	
		totalEncoder=0;
		totalEncoderLeft=0;
		EnqueueString("qwer");
		EnqueueChar('l');
		EnqueueChar('\n');
		EnqueueInt16((uint16_t)x);
		EnqueueInt16((uint16_t)y);
		if(carDirection==-1)EnqueueInt16((uint16_t)(carDirection+1));
				else EnqueueInt16((uint16_t)carDirection);
		EnqueueChar(0x1A);
		EnqueueChar('\n');
		EnqueueChar(0xff);
		UART0_C2 |=  UART_C2_TIE_MASK;  //enable transmit interrupt for start sendding
 }

 
 //****************** Maintain a straight line **************************//
 void TicksFix(){
	 //calculate the different between the motors
	 int error=encouderCounterRight-encouderCounterLeft;
	//set the new speed and zero the encoders
	 speedset(1,TPM1_C0V+error*KP);
	 encouderCounterRight=0;
	 encouderCounterLeft=0;
 }
 
 //****************** use the end wall to fix the angle using IR **************************// NOT USED!!!!
void fixDeg(){
	adcMux=0;
	ADC0_SC1A=ADC_SC1_ADCH(7) |ADC_SC1_AIEN_MASK;
	delay(30);
	ADC0_SC1A=ADC_SC1_ADCH(6) |ADC_SC1_AIEN_MASK;
	delay(30);
	int tempLeft=DistanceMeasuring1(leftIr);
	int tempRight=DistanceMeasuring2(rightIr);
	if(tempLeft>tempRight){ //right closer to wall
		while((tempLeft>tempRight)){
			GPIOC_PSOR=0X0000400;
			speedset(1,1500);
			adcMux=0;
			ADC0_SC1A=ADC_SC1_ADCH(7) |ADC_SC1_AIEN_MASK;
			delay(30);
			ADC0_SC1A=ADC_SC1_ADCH(6) |ADC_SC1_AIEN_MASK;
			delay(30);
			tempLeft=DistanceMeasuring1(leftIr);
			tempRight=DistanceMeasuring2(rightIr);
		}
		speedset(1,300);
		GPIOC_PCOR=0X0000400;
	}
	else if(tempLeft<tempRight){ //left closer to wall
		while((tempLeft<tempRight)){
			GPIOC_PSOR=0X0000040;
			speedset(0,1500);
			adcMux=0;
			ADC0_SC1A=ADC_SC1_ADCH(7) |ADC_SC1_AIEN_MASK;
			delay(30);
			ADC0_SC1A=ADC_SC1_ADCH(6) |ADC_SC1_AIEN_MASK;
			delay(30);
			tempLeft=DistanceMeasuring1(leftIr);
			tempRight=DistanceMeasuring2(rightIr);
		}
		speedset(0,300);
		GPIOC_PCOR=0X0000040;
	}
}
  
