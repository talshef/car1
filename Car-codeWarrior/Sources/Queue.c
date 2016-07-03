/*
 * Queue.c
 *
 *  Created on: May 10, 2016
 *      Author: talsheff
 */
#include "TFC.h"
#include "mcg.h"

/*Queue - Linked List implementation*/
#include<stdio.h>
#include<stdlib.h>
struct Node {
	char data;
	struct Node* next;
};

struct Node* front = NULL;
struct Node* rear = NULL;
// To Enqueue an float

void EnqueueFloat(float x) {
	int i;
	char *addrStart=(unsigned char *)&x;
	for(i=0;i<4;i++){
		EnqueueChar(*(addrStart+i));
	}
}
void EnqueueString(char* str){
	volatile unsigned char i;
	for (i=0 ; str[i] ; i++){
		EnqueueChar(str[i]) ;
		
	}
}
void EnqueueInt16(uint16_t sample){
	char* temp=&sample;
		EnqueueChar(*(temp)) ;
	
}


void EnqueueArray16bit(uint16_t arr){
	volatile unsigned char i;
	char* temp=&arr;
	int j;
	for (i=0 ; i<88 ; i++){
		EnqueueChar(*(temp+i)) ;
		UART0_C2 |=  UART_C2_TIE_MASK;
	}
	for(j=0;j<10000000;j++);
	for(j=0;j<10000000;j++);
	for (i=88 ; i<176 ; i++){
			EnqueueChar(*(temp+i)) ;
			UART0_C2 |=  UART_C2_TIE_MASK;
	}
}
void EnqueueChar(char j) {
	struct Node* temp = 
		(struct Node*)malloc(sizeof(struct Node));
	temp->data =j; 
	temp->next = NULL;
	if(front == NULL && rear == NULL){
		front = rear = temp;
		return;
	}
	rear->next = temp;
	rear = temp;
}

// To Dequeue an integer.
char Dequeue() {
	struct Node* temp = front;
	char val;
	if(front == NULL) {
		//printf("Queue is Empty\n");
		return;
	}
	if(front == rear) {
		temp=front;
		front = rear = NULL;
	}
	else {
		temp=front;
		front = front->next;
	}
	val= temp->data;
	free(temp);
	return val;
	
}

int isEmpty(){
	if(front == NULL) {
		
		return 1;
	}
	else return 0;
}
void freeQue(){
front=NULL;
rear=NULL;
}




