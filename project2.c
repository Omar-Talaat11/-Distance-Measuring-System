#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/* Definitions of maximum and minimum values of timer values */
#define Max 60
#define Min 0

/* Declaration of global variables of timer values */
unsigned char s=Min,m=Min,h=Min,c=0,preset=0;

/* Functions Declarations */
void Timer1_init(void); //initiation of timer1 to ctc mode
void enb_seg(unsigned char a); //function that enables one 7 segment and disables the others
void INT0_init(void); //Initiation Interrupt 0
void INT1_init(void); //Initiation Interrupt 1
void INT2_init(void); //Initiation Interrupt 2
void timer0_init_comp(void); //initiation of timer0 to ctc mode



int main(void)
{
	DDRD&=~((1<<2)|(1<<3)|(1<<4));
	PORTD=0;
	DDRB &=~(1<<2);
	DDRC|=0x0F;
	DDRA|=0x3F;

	preset=10;

	Timer1_init();
	INT0_init();
	INT1_init();
	INT2_init();
	timer0_init_comp();

	for(;;)
	{
		/* Reseting Variables when reaching maximum values */

		if(s==Max)
		{
			m++;
			s=Min;
		}
		if(m==Max)
		{
			h++;
			m=Min;
		}
		if(h==99)
		{
			h=Min;
		}


		enb_seg(0);
		PORTC = (PORTC&0xF0)|((s%10)& 0x0F);
		_delay_ms(4);

		enb_seg(1);
		PORTC = (PORTC&0xF0)|((s/10)& 0x0F);
		_delay_ms(4);

		enb_seg(2);
		PORTC = (PORTC&0xF0)|((m%10)& 0x0F);
		_delay_ms(4);

		enb_seg(3);
		PORTC = (PORTC&0xF0)|((m/10)& 0x0F);
		_delay_ms(4);

		enb_seg(4);
		PORTC = (PORTC&0xF0)|((h%10)& 0x0F);
		_delay_ms(4);

		enb_seg(5);
		PORTC = (PORTC&0xF0)|((h/10)& 0x0F);
		_delay_ms(4);

	}
	return 0;
}


void Timer1_init(void)
{
	/*
	 * Fcpu = 1Mhz
	 * Prescaler = 64
	 * Ttimer = 64*10^-6
	 * OCR1A = 15625
	 */
	TCCR1A=(1<<FOC1A);
	TCCR1B=(1<<WGM12)|(1<<CS10)|(1<<CS11);
	TIMSK|=(1<<OCIE1A);
	TCNT1=0;
	OCR1A=15625;
	SREG|=(1<<7);
}



ISR(TIMER1_COMPA_vect)
{
	/*
	 * Interrupt Service routine of timer1 just increments the stop watch timer values
	 */
	s++;
	TCNT1=0;
}



void enb_seg(unsigned char a)
{
	/* Function takes the seven segment number and enables it */
	PORTA=0;
	PORTA|=(1<<a);
}


void INT0_init(void)
{
	SFIOR&=~(1<<PUD); //Enabling internal pull-up resistors
	PORTD|=(1<<2); //Enabling pull-up resistor at pin 2 of Port D
	MCUCR&=~(1<<ISC00); //Making interrupt triggered on falling edge
	MCUCR|=(1<<ISC01);
	GICR|=(1<<INT0);//enabling interrupt at interrupt 0
}

ISR (INT0_vect)
{ /* Function that makes the timer watch reset */
	s=Min,m=Min,h=Min;
	c=0;
}

void INT1_init(void)
{
	MCUCR|=(1<<ISC11)|(1<<ISC10);//Making interrupt triggered on rising edge
	GICR|=(1<<INT1);//enabling interrupt at interrupt 1
}

ISR (INT1_vect)
{ /* Function that disables clock to timer 1 and timer 0*/
	TCCR1B&=~((1<<CS10)|(1<<CS11)|(1<<CS12));
	TCCR0&=~((1<<CS02)|(1<<CS00));
}

void INT2_init(void)
{
	PORTB|=(1<<2);//Enabling pull-up resistor at pin 2 of Port B
	MCUCSR&=~(1<<ISC2);//Making interrupt triggered on falling edge
	GICR|=(1<<INT2);//enabling interrupt at interrupt 2
}

ISR (INT2_vect)
{ /* Function that enables clock to timer 1 and timer 0*/
	TCCR1B|=(1<<CS10)|(1<<CS11);
	TCCR0|=((1<<CS02)|(1<<CS00));
}


void timer0_init_comp(void)
{
	/*
	 * Function that initiates timer 0
	 * Fcpu = 1Mhz
	 * prescaler = 1024
	 * Ttimer = 1024*10^-6
	 * Compare value = 244 for quarter second
	 */

	TCCR0=(1<<FOC0)|(1<<CS02)|(1<<CS00)|(1<<WGM01);
	TCNT0=0;
	OCR0=244;
	TIMSK|=(1<<OCIE0);
	SREG|=(1<<7);
}

ISR(TIMER0_COMP_vect)
{ /* Function that counts time using timer 0 until reaching preset value and trigger LED */
	c++;
	if(c>=4*preset)
	{
		if(!(c%2))
		{
			PORTD^=(1<<4);
		}
	}
	if(c==68)
	{
		TCCR0&=~((1<<CS02)|(1<<CS00));
	}
}
