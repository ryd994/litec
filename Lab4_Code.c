#include <c8051_SDCC.h>
#include <stdio.h>
#include <stdlib.h>
#include <i2c.h>

unsigned int SERVO_PW = 2735;
unsigned int SERVO_MAX = 3335;
unsigned int SERVO_MIN = 2185;
unsigned int heading;
unsigned int range;
int compass_adj = 0;
int range_adj = 0;

unsigned int PCACounter = 0;
unsigned int motor_min,motor_max;

__sbit __at 0xB7 RUN;
	

void Port_Init(void)
void XBR0_Init(void)
void SMBus_Init(void)
void ADC_Init(void)
void Interrupt_Init(void)
void PCA_Init(void)
void Output_Init(void)
	
void PCA_ISR( void ) __interrupt 9;
	
int Update_Value( int Constant, unsigned char incr, int maxval, int minval );
unsigned char read_AD_input( unsigned char n );
int read_compass( void );
void set_servo_PWM( void );
int read_ranger( void );
void set_drive_PWM( void );
int pick_heading(void);
int pick_range(void);
	
	
void main(void) {
	//Local Variables
	unsigned char run_stop
	//Initialization Functions
	Sys_Init();
	putchar(' ');
	Port_Init();
	XBR0_Init();
	SMBus_Init();
	ADC_Init();
	Interrupt_Init();
	PCA_Init();
	Output_Init();
	
	while ( 1 ) {
		run_stop = 0;
		while ( !RUN ) {
			if (run_stop == 0) {
				desired_heading = pick_heading();
				desired_range = pick_range();
				run_stop = 1;
			}
		}
		
		if ( PCACounter % 2 == 0 ) {
			heading = read_compass();
			set_servo_PWM();
			
		}
		
		if ( PCACounter % 4 == 0 ){
			range = read_ranger();
			set_range_adj();
			new_range = 0;
		}
		
		if( PCACounter % 20 == 0 ) {
			//LCD code TODO
		}
	}
}


//Functions

void Port_Init(void) {
	P1MDOUT = 0x05; //set output pin for CEX0 in push-pull mode
	P3MDOUT &= ~0x80; //set P3.7 to open drain (input)
	P3 |= 0x80; //Set P3.7 to high impedance
}

void XBR0_Init(void) {
	XBR0 = 0x27;
}

void Interrupt_Init(void){
	EIE1 |= 0x08;
	EA = 1;
}

void PCA_Init(void){
	PCA0MD &= 0xF1;
	PCA0MD |= 0x01;
	PCA0CPM0 = 0xC2;
	PCA0CPM2 = 0xC2;
	PCA0CN = 0x40;
	PCA0 = 28671;
}

void SMBus_Init(void){
	SMB0CR = 0x93;
	SMB0CN |= 0x40;
}

void ADC_Init(void){
	REF0CN = 0x03; //
	ADC1CN = 0x80; //
	ADC1CF &= ~0x03; //Clears lowest 2 bits
	ADC1CF |= 0x01; //Gain 1
}

void motor_init(void) {
	PCA0CP2	=62771; 	//1.5ms PW

	PCACounter = 0;
	while (PCACounter<50) {};
	
	motor_min = 62034;	//1.9ms PW
	motor_max = 63508;	//1.1ms PW

	printf("Setting forward speed limit, press d when done.\n\r");
	printf("press d when done\n\r");
	while (set_motor_speed()!='d') {};
	motor_min = PCA0CP2;

	PCA0CP2	=62771;		//1.5ms
	printf("Setting reverse speed limit, press d when done.\n\r");
	printf("press d when done\n\r");
	while (set_motor_speed()!='d') {};
	motor_max = PCA0CP2;

	printf("Setting Finish\n\r");
}

void PCA_ISR(void) __interrupt 9 {
	if ( CF ) {
		PCA0 = 28672;
		CF = 0;
		PCACounter++;
	}
}

int Update_Value( int Constant, unsigned char incr, int maxval, int minval ){
	//Local Variables
	int deflt;
	char input;
	deflt = Constant;
	while ( TRUE ) {
		input = getchar();
		if ( input == 'c' ) { 
			Constant = deflt;
		}
		if ( input == 'i' ) {
			Constant += incr;
			if ( Constant > maxval ) {
				Constant = maxval;
			}
		}
		if ( input == 'd' ) {
			Constant -= incr;
			if (Constant < minval) {
				Constant = minval;
			}
		}
		if ( input == 'u' ) {
			return Constant;
		}
	}
}

unsigned char read_AD_input( unsigned char n ) {
	AMX1SL = n;
	ADC1CN = ADC1CN & ~0x20;
	ADC1CN = ADC1CN | 0x10;
	while ((ADC1CN & 0x20) == 0x00);
	return ADC1;
}

int read_compass( void ){
	unsigned char addr = 0xC0;
	unsigned char Data[2];
	unsigned int heading;
	i2c_read_data(addr, 2, Data, 2);
	heading =(((unsigned int)Data[0] << 8) | Data[1]);
	return heading;
}

void set_servo_PWM( void ){
	set PCACP0 to the correct pulsewidth
	PCACP0 = 0xFFFF - PW
}

unsigned char set_motor_speed(void) {
	//motor_min and motor_max must be initialized before calling
	// return user inputed char
	unsigned char user_input;
	printf("press f for forward, s for reverse(slower)\n\r");
	user_input = getchar();
	switch(user_input) {
		case 'f':
			if (PCA0CP2 > motor_min) PCA0CP2-=10; 
			break;
		case 's':
			if (PCA0CP2 < motor_max) PCA0CP2+=10;
			break;
	}
	return user_input;
}

