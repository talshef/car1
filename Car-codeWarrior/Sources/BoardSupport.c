#include "TFC.h"
#include "mcg.h"

#define MUDULO_REGISTER  0x2EE0

uint16_t IrSampels[90];
char maze[16][20];
int totalEncoder=0;
int totalEncoderLeft=0;
double x=41.5;
double y=2;
int leftIr=0;
int rightIr=0;
double BattSamp=0;
int Irdistance[2][136];
int sonicCapture=0;
double sonicRightDistance=0;
double sonicLeftDistance=0;
int rightCaptureFlag=0;
int leftCaptureFlag=0;
double carDirection=0;
int encouderCounterLeft=0;
int encouderCounterRight=0;
int adcMux=0;
int directionCount=0;
int state=9;
int pitVal=0;
int strightTicks=0;
int battSample=0;

// set I/O for switches and LEDs
void InitGPIO()
{
	//enable Clocks to all ports - page 206, enable clock to Ports
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;

	//GPIO Configuration - LEDs - Output
	PORTD_PCR1 = PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;  //Blue
	GPIOD_PDDR |= BLUE_LED_LOC; //Setup as output pin	
	PORTB_PCR18 = PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //Red  
	PORTB_PCR19 = PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK; //Green
	GPIOB_PDDR |= RED_LED_LOC + GREEN_LED_LOC; //Setup as output pins
	
	//GPIO Configuration - Pushbutton - Input
	
}
//-----------------------------------------------------------------
// DipSwitch data reading
//-----------------------------------------------------------------
uint8_t TFC_GetDIP_Switch()
{
	uint8_t DIP_Val=0;
	
	DIP_Val = (GPIOC_PDIR>>4) & 0xF;

	return DIP_Val;
}
//----------------------------------------------------------------
//TPM setup
//----------------------------------------------------------------

void TPMSetup(){
	
	//TPM0
	TPM0_SC = 0;// to ensure that the counter is not running
	TPM0_SC|=TPM_SC_PS(6) |  TPM_SC_CMOD(1);//|TPM_SC_TOIE_MASK ;
	TPM0_MOD=MUDULO_TPM;
	TPM0_CONF = 0;
	//TPM1
	TPM1_SC = 0; // to ensure that the counter is not running
	TPM1_SC |= TPM_SC_PS(6) | TPM_SC_CMOD(1);//prescaler 64;
	TPM1_MOD = MUDULO_TPM; // PWM frequency of 40Hz=24000000/(64*40) 
	TPM1_CONF = 0; 

	//TPM2
	TPM2_SC = 0; // to ensure that the counter is not running
	TPM2_SC |= TPM_SC_PS(6) | TPM_SC_CMOD(1);//prescaler 64;
	TPM2_MOD = MUDULO_TPM; // PWM frequency of 40Hz=24000000/(64*40) 
	TPM2_CONF = 0; 
}




//-----------------------------------------------------------------
// Clock Setup
//-----------------------------------------------------------------
void ClockSetup(){
	
		    
	
	    pll_init(8000000, LOW_POWER, CRYSTAL,4,24,MCGOUT); //Core Clock is now at 48MHz using the 8MHZ Crystal
		
	    //Clock Setup for the TPM requires a couple steps.
	    //1st,  set the clock mux
	    //See Page 124 of f the KL25 Sub-Family Reference Manual
	    SIM_SOPT2 |= SIM_SOPT2_PLLFLLSEL_MASK;// We Want MCGPLLCLK/2=24MHz (See Page 196 of the KL25 Sub-Family Reference Manual
	    SIM_SOPT2 &= ~(SIM_SOPT2_TPMSRC_MASK);
	    SIM_SOPT2 |= SIM_SOPT2_TPMSRC(1); //We want the MCGPLLCLK/2 (See Page 196 of the KL25 Sub-Family Reference Manual
		//Enable the Clock to the TPM0 and PIT Modules
		//See Page 207 of f the KL25 Sub-Family Reference Manual
		SIM_SCGC6 |= SIM_SCGC6_TPM0_MASK + SIM_SCGC6_TPM2_MASK+SIM_SCGC6_TPM1_MASK;
	    // TPM_clock = 24MHz , PIT_clock = 48MHz
	    
		
			   
}
//-----------------------------------------------------------------
// PIT - Initialisation
//-----------------------------------------------------------------
void InitPIT(){
	//PIT_MCR =PIT_MCR_MDIS_MASK;
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK; //Enable the Clock to the PIT Modules
	// Timer 0
	PIT_LDVAL0 = 960000; // setup timer 0 for 0.04sec counting period
	PIT_TCTRL0 = PIT_TCTRL_TEN_MASK | PIT_TCTRL_TIE_MASK; 
	PIT_MCR = 0;
	PIT_TCTRL0=0;
	
	
	enable_irq(INT_PIT-16); //  //Enable PIT IRQ on the NVIC
	set_irq_priority(INT_PIT-16,0);  // Interrupt priority = 0 = max
}
//-----------------------------------------------------------------
//DTF - Initialisation
//-----------------------------------------------------------------
void dtf_init(){
	PORTD_PCR7 = PORT_PCR_MUX(1); // assign PTD7 as GPIO
	GPIOD_PDDR &= ~PORT_LOC(7);  // PTD7 is Input
	PORTD_PCR7 |= PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_PFE_MASK | PORT_PCR_IRQC(0x0a);
	enable_irq(INT_PORTD-16); // Enable Interrupts 
	set_irq_priority (INT_PORTD-16,0);  // Interrupt priority = 0 = max
	PORTC_PCR11= PORT_PCR_MUX(1)|PORT_PCR_PS_MASK| PORT_PCR_PE_MASK;
	PORTC_PCR12= PORT_PCR_MUX(1)|PORT_PCR_PS_MASK| PORT_PCR_PE_MASK;
	PORTC_PCR13= PORT_PCR_MUX(1)|PORT_PCR_PS_MASK| PORT_PCR_PE_MASK;
	PORTA_PCR17= PORT_PCR_MUX(1)|PORT_PCR_PS_MASK| PORT_PCR_PE_MASK;
	GPIOC_PDDR &= ~(PORT_LOC(11)|PORT_LOC(12)|PORT_LOC(13));
	GPIOA_PDDR &= ~(PORT_LOC(17));
}
void delay(int milisec){
	int i;
	int ticks=milisec*4000;
	for(i=0;i<ticks;i++);
}
