#include <avr/io.h>
#include "m_general.h"
#include "m_bus.h"
#include "m_rf.h"
#include "m_wii.h"
#include "m_usb.h"

#define CHANNEL 1
#define RXADDRESS 0x28
#define PACKET_LENGTH 12
#define COMM 0xA0
#define PLAY 0xA1
#define GOALA 0xA2
#define GOALB 0xA3
#define PAUSE 0xA4
#define HALFTIME 0xA6
#define GAME_OVER 0xA7


double A = 115*94/29;
double B = -115*94/29;
char buffer[PACKET_LENGTH];
int data[12];
double south[2];
double north[2];
double east[2];
double west[2];
double orientation =0;
double x =0;
double y=0;
double goalx = 0;
double goaly = 0; 
double xoffset = 12;
double yoffset = 15; 
int F0 = 0;
int F1 = 0;
int F4=0;
int F5=0;
int F6=0;
int F7=0;
double ang = 0;
double reference = 0; 
int smallest = 2000;  
int secsmall = 2000;
int smallpos = 10;
int secpos = 10;
int indexi[6] = {0,0,0,3,3,6};
int indexj[6] = {3,6,9,6,9,9}; 
double real_dis[4] = {44.6,89.1, 68.6, 54.85};
	int main(void)
	{  
		//m_bus_init();
		char blah = m_wii_open();
		m_clockdivide(0);  //set system clock to 16M
		m_rf_open(CHANNEL,RXADDRESS,PACKET_LENGTH); //configure mRF
		
		
		OCR1A = 1000;
		ICR3 = 1000; 
		
		//Timer 1
		clear(TCCR1B,CS12); //set  timer1 clock cycle to divide by 1
		clear(TCCR1B,CS11);   //
		set(TCCR1B,CS10);  //
		
		set(TCCR1B,WGM13); //set mode to 15 up to OCR1A
		set(TCCR1B,WGM12);
		set(TCCR1A,WGM11);
		set(TCCR1A,WGM10);
		
		set(TCCR1A,COM1B1); //set to clear
		clear(TCCR1A,COM1B0);

		//Time 3
		clear(TCCR3B,CS32); //set  timer3 clock cycle to divide by 1
		clear(TCCR3B,CS31);   //
		set(TCCR3B,CS30);  //
		
		
		set(TCCR3B,WGM33); //set mode to 14 up to ICR3
		set(TCCR3B,WGM32);
		set(TCCR3A,WGM31);
		clear(TCCR3A,WGM30);
		
		set(TCCR3A,COM3A1); //set to clear
		clear(TCCR3A,COM3A0);
		
		
		//motor pins
		set(DDRB,5); //set pinB5 to output //left
		set(DDRC,7); //set pinC7 to output  //right
		set(DDRB,6); //left motor
		set(DDRC,6);
		
		set(PORTC,7); // set motors to move in the same direction
		set(PORTB,5);

		
		//set(EIMSK,INT2);
		//sei();
		
		//ADC Conversion
		 m_disableJTAG();//allowing gpio of F pins
		
		clear(ADMUX,REFS1);  // set voltage reference to Vcc
		set(ADMUX,REFS0);    //
		
		set(ADCSRA,ADPS2);  //set ADC Prescaler to divide 16 from default clock of 2Mhz
		clear(ADCSRA,ADPS1); //
		clear(ADCSRA,ADPS0); //
		
		set(DIDR0,ADC0D);  //disabling digital inputs of F0
		set(DIDR0,ADC1D);  //disabling digital inputs of F1
		set(DIDR0,ADC4D);  //disabling digital inputs of F4
		set(DIDR0,ADC5D);  //disabling digital inputs of F5
		set(DIDR0,ADC6D);  //disabling digital inputs of F6
		set(DIDR0,ADC7D);  //disabling digital inputs of F7

		
		set(ADCSRA,ADEN); //enable ADC subsystem
		buffer[0] = PLAY; 
		while(1){
			cli();
			m_wii_read(data);
			sei();

			
			switch (buffer[0])
			{
			case COMM:
				stop(); 
				m_green(ON);
				m_wait(1000);
				m_green(OFF);
 				//position(); 
				if(x<0){      //locate opponent's goal
				goalx = A;}
				else{
				goalx = B;}
				break;
				
			case PLAY:
				while(buffer[0] == PLAY){		
					adc();
					cli();
					m_wii_read(data);
					sei();
					findPuck();
					//findSmallest();
					//findSecond();
					//if(m_usb_rx_available()){
						//m_usb_tx_int(F0);
						//m_usb_tx_string("    ");
						//m_usb_tx_int(F1);
						//m_usb_tx_string("    ");
						//m_usb_tx_int(F4);
						//m_usb_tx_string("    ");
						//m_usb_tx_int(F5);
						//m_usb_tx_string("    ");
						//m_usb_tx_int(F6);
						//m_usb_tx_string("    ");
						//m_usb_tx_int(F7);
						//m_usb_tx_string("    ");
						//m_usb_tx_int(smallpos);
						//m_usb_tx_string("    ");
						//m_usb_tx_int(secpos);
						//m_usb_tx_string("    ");
						//
						//m_usb_tx_string("\n");
					//}
					//m_wait(1000);
				}
				break;

			case GOALA:
				stop();		
				break;
			case GOALB:
				stop(); 
				break; 
			case PAUSE:
				stop(); 
				break;
			case HALFTIME:
				stop(); 
				if(goalx == 115*94/29){
					goalx = -115*94/29;}
				else{
					goalx = 115*94/29; }
				break; 
				
			case GAME_OVER:
				stop();
				break; 
				

			default:
				stop(); 
				break; 
			
			
		}		

	}
}

ISR(INT2_vect){
	m_rf_read(buffer, PACKET_LENGTH);
	m_red(TOGGLE);
		
}
	
double length(int a, int b, int c, int d){
		double ans = sqrt(((a-c)*(a-c)+(b-d)*(b-d)));
		return(ans);
}
	
void position(void){
double dis[6] = {0,0,0,0,0,0};
dis[0] = length(data[0],data[1],data[3],data[4]);
dis[1] = length(data[0],data[1],data[6],data[7]);
dis[2] = length(data[0],data[1],data[9],data[10]);
dis[3] = length(data[3],data[4],data[6],data[7]);
dis[4] = length(data[3],data[4],data[9],data[10]);
dis[5] = length(data[6],data[7],data[9],data[10]);
		int valid[4] = {10,10,10,10};
		int posi[4] = {5,5,5,5};
		int posj[4] = {5,5,5,5};

		for(int j=0; j<4; j++){
			for(int k = 0; k<6; k++){
				if((dis[k]>real_dis[j]-5)&&(dis[k]<real_dis[j]+5)){
					valid[j] = j;
					posi[j] = indexi[k];
					posj[j] = indexj[k]; 
				}
			}
		}		
		if(valid[0] == 0){
			if(valid[1]==1){
				if(posi[0] == posi[1] ||posi[0] == posj[1]){
					north[0] = data[posj[0]];
					north[1] = data[posj[0]+1];
					if(posi[0] == posi[1]){
						south[0] = data[posj[1]];
						south[1] = data[posj[1]+1];}
					else{
						south[0] = data[posi[1]];
						south[1] = data[posi[1]+1];}
				}	
				else{
					north[0] = data[posi[0]];
					north[1] = data[posi[0]+1];
					if(posj[0] == posi[1]){
						south[0] = data[posj[1]];
						south[1] = data[posj[1]+1];}
					else{
						south[0] = data[posi[1]];
						south[1] = data[posi[1]+1];}
				}				
			}
				
			else if(valid[3] == 3){
				if(posi[0] == posi[3] || posi[0] == posj[3]){
					north[0] = data[posi[0]];
					north[1] = data[posi[0]+1];
					east[0] = data[posj[0]];
					east[1] = data[posj[0]+1];
				}
				else{
					north[0] = data[posj[0]];
					north[1] = data[posj[0]+1];
					east[0] = data[posi[0]];
					east[1] = data[posi[0]+1];
				}
				double angNE = 0.4589 +3.1415/2 +atan2(east[0] - north[0], east[1]- north[1]);
				south[0] = north[0] - 99.43*sin(angNE);
				south[1] = north[1] - 99.43*cos(angNE);
						
					
			}
			else{
				north[0] = 0;
				south[0] = 0; 
			}
		}
			
		else if(valid[1] == 1){
			if(valid[2] == 2){
				if(posi[1] == posi[2] || posi[1] == posj[2]){
					south[0] = data[posi[1]];
					south[1] = data[posi[1]+1];
					east[0]= data[posj[1]];
					east[1] = data[posj[1]+1];
				}
				else{
					south[0] = data[posj[1]];
					south[1] = data[posj[1]+1];
					east[0] = data[posi[1]];
					east[1] = data[posi[1]+1];
				}
				double angSE = 1.1060 + atan2(east[1] - south[1], east[0]-south[0]);
				north[0] = south[0]+99.43*sin(angSE);
				north[1]= south[1] - 99.43*cos(angSE);
						
			}
		}			
		else if (valid[2] == 2 && valid[3] == 3){
			if(posi[2] == posi[3] || posi[2] == posj[3]){
				south[0] = data[posj[2]];
				south[1] = data[posj[2] +1];
				if(posi[2] == posi[3]){
					north[0] = data[posj[3]];
					north[1] = data[posj[3]+1];}
				else{
					north[0] = data[posi[3]];
					north[1] = data[posi[3]+1];}
			}
			else{
				south[0] = data[posi[2]];
				south[1]=data[posi[2] +1];
				if(posj[2] == posi[3]){
					north[0] = data[posj[3]];
					north[1] = data[posj[3]+1];}
				else{
					north[0] = data[posi[3]];
					north[1] = data[posi[3]+1];}
			} 
		}
			
		else{
			south[0] =0;
			north[0] = 0; 
		}
			
		if(north[0] != 0 &&  south[0] != 0){
			orientation = atan2(north[0]-south[0],north[1]-south[1]);
			x = (512-(north[0]+south[0])/2)*cos(orientation) - (384 -(north[1]+south[1])/2)*sin(orientation) - xoffset;
			y = (512-(north[0]+south[0])/2)*sin(orientation) + (384 -(north[1]+south[1])/2)*cos(orientation) -yoffset; 
		}
		
	
}



void findSmallest (void){
	smallest = F1;
	smallpos = 1;
	if(smallest>F4){
		smallest = F4;
		smallpos = 4;}
	if(smallest>F5){
		smallest = F5;
		smallpos = 5;}
	if(smallest>F6){
		smallest = F6;
		smallpos = 6;}	
}



void findSecond(void){
		secsmall = 2000; 
		if(smallpos != 1 && secsmall>F1){
			secsmall = F1;
			secpos = 1;
		}
		if(smallpos != 4 && secsmall>F4){
			secsmall = F4;
			secpos = 4;
		}
		if(smallpos != 5 && secsmall>F5){
			secsmall = F5;
			secpos = 5;
		}
		if(smallpos != 6 && secsmall>F6){
			secsmall = F6;
			secpos = 6;
		}
}



void findPuck(void){

	findSmallest();
	findSecond();
	position(); 
	if(smallpos == 0) {
		clear(PORTC,7); // set motors to move in the same direction
		clear(PORTB,5);
		fullSpeed();}
	else if(smallpos == 7){
		set(PORTC,7); // set motors to move in the same direction
		set(PORTB,5);
		fullSpeed();
	}
	else if ((smallpos == 1 || (smallpos == 1 && secpos== 4) ||(smallpos == 4 && secpos== 1))){
			if(y>-30){
				clear(PORTC,7); // set motors to move in the same direction
				clear(PORTB,5);
				fullSpeed();}
			else{
				stop();} }		

	else if (smallpos == 6 || (smallpos == 6 && secpos== 5) ||(smallpos == 5 && secpos== 6) ){
			if(y<80){
				set(PORTC,7); // set motors to move in the same direction
				set(PORTB,5);
				fullSpeed(); }
			else{
				stop();} 
			}
			
		
		
	else if ((smallpos == 4 && secpos== 5)||(smallpos == 5 && secpos== 4)){
		stop();
	}
	else
		stop();

}


void adc(void){
	//F0
	clear(ADCSRB, MUX5);
	clear(ADMUX, MUX2);
	clear(ADMUX, MUX1);
	clear(ADMUX, MUX0);

	set(ADCSRA, ADSC);

	while(!check(ADCSRA, ADIF));
	F0 = ADC;
	set(ADCSRA, ADIF);



	//F1
	clear(ADCSRB, MUX5);
	clear(ADMUX, MUX2);
	clear(ADMUX, MUX1);
	set(ADMUX, MUX0);

	set(ADCSRA, ADSC);

	while(!check(ADCSRA, ADIF));
	F1 = ADC;
	set(ADCSRA, ADIF);


	//F4
	clear(ADCSRB, MUX5);
	set(ADMUX, MUX2);
	clear(ADMUX, MUX1);
	clear(ADMUX, MUX0);

	set(ADCSRA, ADSC);

	while(!check(ADCSRA, ADIF));
	F4 = ADC;
	set(ADCSRA, ADIF);


	//F5
	clear(ADCSRB, MUX5);
	set(ADMUX, MUX2);
	clear(ADMUX, MUX1);
	set(ADMUX, MUX0);

	set(ADCSRA, ADSC);

	while(!check(ADCSRA, ADIF));
	F5 = ADC;
	set(ADCSRA, ADIF);
	
	//F6
	clear(ADCSRB, MUX5);
	set(ADMUX, MUX2);
	set(ADMUX, MUX1);
	clear(ADMUX, MUX0);

	set(ADCSRA, ADSC);

	while(!check(ADCSRA, ADIF));
	F6 = ADC;
	set(ADCSRA, ADIF);


	//F7
	clear(ADCSRB, MUX5);
	set(ADMUX, MUX2);
	set(ADMUX, MUX1);
	set(ADMUX, MUX0);

	set(ADCSRA, ADSC);

	while(!check(ADCSRA, ADIF));
	F7 = ADC;
	set(ADCSRA, ADIF);
}


void stop(void){
	OCR1B = 0; 
	OCR3A = 0; 
}
	

void fullSpeed(void){
	OCR1B = 1000;
	OCR3A = 1000; 
}
