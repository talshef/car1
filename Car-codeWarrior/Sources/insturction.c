/*
 * insturction.c
 *
 *  Created on: Jun 7, 2016
 *      Author: talsheff
 */


# include "TFC.h"
#include "adc.h"
#include <stdio.h>
#include<stdlib.h>
struct Instruction drivingInstructions[50];
int InstructIndex=0;



//******************function for pass the half circle**************************//
void findHole(){
	IrScan();
	int maxLeft=0;
	int maxIndexLeft=0;
	int maxRight=0;
		int maxIndexRight=0;
		int i;
	//find the max index in each side
	for(i=43;i>=0;i--){
		if (maxLeft<IrSampels[i]){
			maxLeft=IrSampels[i];
			maxIndexLeft=i;
		}
	}
	for(i=44;i<88;i++){
			if (maxRight<IrSampels[i]){
				maxRight=IrSampels[i];
				maxIndexRight=i;
			}
		}
	//choose which side- depends on who closer to the center
	int maxIndex=maxIndexLeft;
	int max=maxLeft;
	if((maxIndexRight-44)<(43-maxIndexLeft)&&maxRight==150) {
		max=maxRight;
		maxIndex=maxIndexRight;
	}
	//do optimisation to the center of the hole
	int counterLeft=1;
	while(max-10<=IrSampels[maxIndex-counterLeft]){
		counterLeft++;
	}
	int counterRight=1;
	while(max-10<=IrSampels[maxIndex+counterRight]){
		counterRight++;
	}
	double deg;
	int avrIndex=(counterRight+counterLeft)/2+maxIndex-counterLeft;
	//calculate the angle- convert the servos angle to the cars angle
	if(avrIndex<43){
		deg=-90+57.3*atan((cos(0.01745*(43-avrIndex)*2.51)*IrSampels[avrIndex]/5+3)/(0.5+sin(0.01745*(43-avrIndex)*2.51)*IrSampels[avrIndex]/5));
	}
	else {
		deg=90-57.3*atan((cos(0.01745*(avrIndex-44)*2.27)*IrSampels[avrIndex]/5+3)/(0.5+sin(0.01745*(avrIndex-44)*2.27)*IrSampels[avrIndex]/5));
	}
	int index=0;
	//plan the instruction for passin the half circle
	drivingInstructions[index].opp='t';
	drivingInstructions[index].num=(int)deg;
	index++; 
	drivingInstructions[index].opp='o';
	drivingInstructions[index].num=2500;
	index++; 
	drivingInstructions[index].opp='f';
	drivingInstructions[index].num=0;
	index++; 
	InstructIndex=0;
	state=9;
}

//******************init the maze**************************//
void initMaze(){
	int i,j;
	int xpath=x/6+1;
	int ypath=(y-2)/6;
	if(ypath>2){
		for(j=ypath;j<19;j++){
			maze[0][j]='W';
			maze[15][j]='W';
			for(i=1;i<15;i++)
				maze[i][j]='_';
		}
	}
	else{
		for(j=0;j<19;j++){
			maze[0][j]='W';
			maze[15][j]='W';
			for(i=1;i<15;i++)
				maze[i][j]='_';
		}
	}	
	
	for(i=0;i<16;i++)
		 maze[i][19]='W';
	maze[7][19]='t';
	maze[8][19]='t';
	
	
	
}

//******************function that calculate route**************************//
 findPath(){
	 //sendMaze();
	 int xpath=x/6+1;
	 	 int ypath=(y-2)/6;
	 	maze[xpath][ypath]='r';
	 	 //check if the car block from all the three direction if so drive backward
	 	if(( maze[xpath-1][ypath]=='H' || maze[xpath-1][ypath]=='W' )&&(maze[xpath][ypath+1]=='H'||maze[xpath][ypath+1]=='W')&&(maze[xpath+1][ypath]=='H'||maze[xpath+1][ypath]=='W')){
	 		strightTicks=600;
	 		startMotorsReverse(6000);
	 		state=2;
	 		return;
	 	}
	 char instruction='A';
	 int counter=0;
	 int index=0;
	 int CurrentDirection=carDirection;
	 while((maze[xpath][ypath+1]!='t')){
		 counter=0;
		 // checks if there a free way up north and if so counting how many squares
		 if(maze[xpath][ypath+1]=='_'){
			 while(maze[xpath][ypath+1]=='_'){
				 maze[xpath][ypath+1]=instruction;
				 ypath++;
				 counter++;
			 }
			 if(maze[xpath][ypath+1]=='H'){
				 drivingInstructions[index].opp='h';
			 	 drivingInstructions[index].num=(counter)*650-550;
			 }
			 else{
				 drivingInstructions[index].opp='d';
				 drivingInstructions[index].num=(counter-1)*650; 
			 }
			 counter=0;
			 index++;
		  }
		 //if up north blocked and the car still not in the last line
		  else if (ypath<17){
			 int leftCounter=0,rightCounter=0;
			 int leftFlag=0,rightFlag=0;
			 leftCounter=0;
			 int tempypath=ypath+1;
			 int tempxpath=xpath;
			 //count the squares from the west until the block from north is end
			 while(maze[tempxpath-1][tempypath]=='H'){
				 leftCounter++;
				 tempxpath--;
			 }
			 //checks its not blocked from the side wall
			 if(maze[tempxpath-1][tempypath]=='W') leftFlag=1;
			 int i;
			 tempypath=ypath;
			 tempxpath=xpath;
			 for(i=0;i<=leftCounter&&!leftFlag;i++){
				 if(maze[tempxpath-1-i][tempypath]=='H'||maze[tempxpath-1-i][tempypath]=='W') 
					 leftFlag=1;
			 }
			 //do the same for the east
			 rightCounter=0;
			 tempypath=ypath+1;
			 tempxpath=xpath;
			 while(maze[tempxpath+1][tempypath]=='H'){
				 rightCounter++;
				 tempxpath++;
			  }
			 if(maze[tempxpath+1][tempypath]=='W') rightFlag=1;
			 tempypath=ypath;
			 tempxpath=xpath;
			 for(i=0;i<=rightCounter&&!rightFlag;i++){
				 if(maze[tempxpath+i+1][tempypath]=='H'||maze[tempxpath+i+1][tempypath]=='W') 
					 rightFlag=1;
			  }
			 //if both side are open choose by the nearest end to the middle
			 if(!rightFlag && !leftFlag){
				 if((7+leftCounter-xpath)<(rightCounter+xpath-7)){
					 drivingInstructions[index].opp='t';
					 drivingInstructions[index].num=-90;
					 index++;		  
					 while(maze[xpath][ypath+1]!='_'){
						 counter++;
						 maze[xpath-1][ypath]=instruction;
						 xpath--;
					 } 
					 if(maze[xpath][ypath+2]=='H'&&maze[xpath-1][ypath]=='_'&&maze[xpath-1][ypath+1]){
						 counter++;
						 maze[xpath-1][ypath]=instruction;
						 xpath--;
					 }
					 drivingInstructions[index].opp='h';
					 drivingInstructions[index].num=(counter+1)*650-500;
					 index++;
					 counter=0;
					 drivingInstructions[index].opp='t';
					 drivingInstructions[index].num=0;
					 index++;
				 }
				 else{
					 drivingInstructions[index].opp='t';
					 drivingInstructions[index].num=90;
					 index++;
					 while(maze[xpath][ypath+1]!='_'){
						  counter++;
						  maze[xpath+1][ypath]=instruction;
						  xpath++;
					 }
					 if(maze[xpath][ypath+2]=='H'&&maze[xpath+1][ypath]=='_'&&maze[xpath+1][ypath+1]){
						 counter++;
						 maze[xpath+1][ypath]=instruction;
						 xpath++;
					 }
					  drivingInstructions[index].opp='h';
					  drivingInstructions[index].num=(counter+1)*650-500;
	 				  index++;
	 				  counter=0;
	 				  drivingInstructions[index].opp='t';
					  drivingInstructions[index].num=0;
					  index++;
				 }
			 }
			 
			 //if only west is free goes left
			 else if(rightFlag && !leftFlag){
				  drivingInstructions[index].opp='t';
				  drivingInstructions[index].num=-90;
				  index++;
				  
				 while(maze[xpath][ypath+1]!='_'){
					 counter++;
					  maze[xpath-1][ypath]=instruction;
					 xpath--;
				 } 
				 if(maze[xpath][ypath+2]=='H'&&maze[xpath-1][ypath]=='_'&&maze[xpath-1][ypath+1]){
					 counter++;
				 	 maze[xpath-1][ypath]=instruction;
				 	 xpath--;
				 }
				 drivingInstructions[index].opp='h';
				 drivingInstructions[index].num=(counter+1)*650-500;
				 index++;
				 counter=0;
				 drivingInstructions[index].opp='t';
				 drivingInstructions[index].num=0;
				 index++;
			  }
			 //if only east is free goes east
			  else if(leftFlag && !rightFlag){
				  drivingInstructions[index].opp='t';
				  drivingInstructions[index].num=90;
				  index++;
				  while(maze[xpath][ypath+1]!='_'){
					  counter++;
					  maze[xpath+1][ypath]=instruction;
					  xpath++;
				  }
				  if(maze[xpath][ypath+2]=='H'&&maze[xpath+1][ypath]=='_'&&maze[xpath+1][ypath+1]){
				 	 counter++;
				 	 maze[xpath+1][ypath]=instruction;
				 	 xpath++;
				 }
				  drivingInstructions[index].opp='h';
				  drivingInstructions[index].num=(counter+1)*650-500;
				  index++;
				  counter=0;
				  drivingInstructions[index].opp='t';
				  drivingInstructions[index].num=0;
				  index++;
			  }
			  else{
				  //if both ways are blocked mark the last square as H clear all A and call the function again
				  if(maze[xpath][ypath]=='r'){
					 RED_LED_ON;
					 strightTicks=600;
					 startMotorsReverse(6000);
					 state=2;
					 return;
				  } 
				  maze[xpath][ypath]='H';
				  int j;
				  int i;
				  for(j=(y-2)/6;j<=ypath;j++){
					  for(i=1;i<15;i++){
						  if(maze[i][j]=='A') maze[i][j]='_';
					  }
				  }
				  return findPath();
			  }
		 }
		 //if the car is in the last line
		 else{
			// direct the car to the right side 
			 if(xpath<7){
				 drivingInstructions[index].opp='t';
				 drivingInstructions[index].num=90;
				 index++; 
				 while(maze[xpath][ypath+1]!='t'){
					 counter++;
					 maze[xpath+1][ypath]=instruction;
					 xpath++;
				  }
				 drivingInstructions[index].opp='h';
				 drivingInstructions[index].num=(counter+1)*650-200;
				 index++;
				 counter=0;
				 drivingInstructions[index].opp='t';
				 drivingInstructions[index].num=0;
				 index++;
			 }
			 // direct the car to the left side 
			 else if(xpath>7) {
				 drivingInstructions[index].opp='t';
				 drivingInstructions[index].num=-90;
				 index++; 
				 while(maze[xpath][ypath+1]!='t'){
					 counter++;
					 maze[xpath-1][ypath]=instruction;
					 xpath--;
				 }
				 drivingInstructions[index].opp='h';
				 drivingInstructions[index].num=(counter+1)*650-200;
				 index++;
				 counter=0;
				 drivingInstructions[index].opp='t';
				 drivingInstructions[index].num=0;
				 index++; 
			 }
		 }
	  }
	 drivingInstructions[index].opp='e';
	 drivingInstructions[index].num=0;
	
	 sendMaze();

	 state=9;
 }

 //******************send the maze to PC**************************//
 void sendMaze(){
	 int i;
	EnqueueString("qwer");
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
	EnqueueChar(0xff);
	 UART0_C2 |=  UART_C2_TIE_MASK;  //enable transmit interrupt for start sendding
	
 }

 //******************find the exit hatch**************************//
 void findEnd(){
	int left;
	int right;
 	Servo1SetPos(9);
 	delay(25);
 	Servo2SetPos(9);
 	ADC0_SC1A=ADC_SC1_ADCH(7);
 	delay(25);
 	left=DistanceMeasuring1(ADC0_RA);
 	ADC0_SC1A=ADC_SC1_ADCH(6);
 	delay(25);
 	right=DistanceMeasuring2(ADC0_RA);
 	//checks if free in front
 	if(left>130&&right>130){
 		strightTicks=3000;
 		startMotors(9375);
 		state=5;
 		return;
 	}
 	//Ir scan to find the hole
 	IrScanEnd();
 	int start=8;
 	int end=79;
 	//find the atart and end index that the range between the will include only the wall 
 	while(IrSampels[start]>130) start++;
 	while(IrSampels[end]>130) end--;
 	int max=0;
 	int maxIndex=0;
 
 	int i;
 	//clean noises
 	for(i=start;i<=end;i++){
		if ((IrSampels[i+1]>140)&&(IrSampels[i-1]>140)){
			IrSampels[i]=150;
		}
		if ((IrSampels[i+1]<130)&&(IrSampels[i-1]<130)){
							IrSampels[i]=20;	
		}
 	 }
 	//find the hole
 	for(i=start;i<=end;i++){
 			if (max<=IrSampels[i]){
 				max=IrSampels[i];
 				maxIndex=i;
 			}
 		}
 	//do optimisation to the center of the hole
 	int counterLeft=1;
 	while(max-5<=IrSampels[maxIndex-counterLeft]){
 		counterLeft++;
 	}
 	int counterRight=1;
 	while(max-5<=IrSampels[maxIndex+counterRight]){
 		counterRight++;
 	}
 	double deg;
 	//calculate the angle- convert the servos angle to the cars angle
 	int avrIndex=(counterRight+counterLeft)/2+maxIndex-counterLeft;
 	if(avrIndex<43){
 		deg=-90+57.3*atan((cos(0.01745*(43-avrIndex)*2.51)*IrSampels[avrIndex]/5+3)/(0.5+sin(0.01745*(43-avrIndex)*2.51)*IrSampels[avrIndex]/5));
 		
 	}
 	else {
 		deg=90-57.3*atan((cos(0.01745*(avrIndex-44)*2.27)*IrSampels[avrIndex]/5+3)/(0.5+sin(0.01745*(avrIndex-44)*2.27)*IrSampels[avrIndex]/5));
 	}
 	int index=0;
 	//start driving in the desirable direction
 	turnPivot((int)deg);
 	strightTicks=3000;
 	startMotors(9375);
 	state=5;
 	
 }
