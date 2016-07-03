
#include "dma.h"
#include "BOARDSUPPORT.h"
extern int ready;


//Not Used In the end!!
void dma_init(void)
{
	
	
	// Enable clocks
	SIM_SCGC6 |= SIM_SCGC6_DMAMUX_MASK;
	SIM_SCGC7 |= SIM_SCGC7_DMA_MASK;
	//**************************dma ch0 for adc*************************************
	// Disable DMA Mux channel first
	DMAMUX0_CHCFG0 = 0x00;
	
	// Configure DMA
	DMA_SAR0 = (uint32_t)&ADC0_RA;
	DMA_DAR0 = (uint32_t)&IrSampels;
	// number of bytes yet to be transferred for a given block - 2 bytes(16 bits)
	DMA_DSR_BCR0 = DMA_DSR_BCR_BCR(180); 
	
	DMA_DCR0 |= ( 
			     DMA_DCR_ERQ_MASK |		// Enable peripheral request
				 DMA_DCR_CS_MASK  |
				 DMA_DCR_SSIZE(2) |		// Set source size to 16 bits
				 DMA_DCR_DINC_MASK|		// Set increments to destination address
				 DMA_DCR_DMOD(8)  |     // Destination address modulo of 128 Bytes
				 DMA_DCR_DSIZE(2));		// Set destination size of 16 bits 
				 
	
	//Config DMA Mux for ADC0 operation, Enable DMA channel and source
	 DMAMUX0_CHCFG0 |=  DMAMUX_CHCFG_SOURCE(40); // Enable DMA channel and set ADC0 as source
	 
	
	//**************************dma ch1 for dac*************************************
/*	DMAMUX0_CHCFG1 = 0x00;
	//Configure DMA 
	DMA_SAR1 = (uint32_t) &value; 
	DMA_DAR1 = (uint32_t) &DAC0_DAT0L ; 	
	
	// number of bytes yet to be transferred for a given block - 2 bytes(16 bits)
	DMA_DSR_BCR1 = DMA_DSR_BCR_BCR(2); 
		
		
	DMA_DCR1 |= (DMA_DCR_EINT_MASK|		// Enable interrupt
				DMA_DCR_ERQ_MASK |		// Enable peripheral request
				DMA_DCR_CS_MASK  |
				DMA_DCR_SMOD(8)  |     // Destination address modulo of 16 Bytes
				DMA_DCR_SINC_MASK|		// Set increments to destination address
				DMA_DCR_SSIZE(2) |		// Set source size to 16 bits				 
				DMA_DCR_DSIZE(2));		// Set destination size of 16 bits 
	
		
		*/
			
}

/*
 * Handles DMA0 interrupt
 * Resets the BCR register and clears the DONE flag
 * */
void DMA0_IRQHandler(void)
{
	/* Enable DMA0*/ 

	while(1);
		
}

void DMA1_IRQHandler(void)
{
	/* Enable DMA0*/ 

	
		DMA_DSR_BCR1 |= 0x1000000u;	// Clear Done Flag
		DMA_DSR_BCR1 |= (((uint32_t)(((uint32_t)(2))<<DMA_DSR_BCR_BCR_SHIFT))&DMA_DSR_BCR_BCR_MASK);
		
}
