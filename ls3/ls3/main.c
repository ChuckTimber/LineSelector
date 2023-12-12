/*
 * ls3.c
 *
 * Created: 2023/11/19 18:23:57
 * Author : Chuck
 */ 

//----- Include Files ---------------------------------------------------------
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>


// PORT definition
//		PMS		Photo MOS Switch
//		LED		Indicator LED
//		SW		Switch
#define PMS_DIR			DDRB
#define LED_DIR			DDRD
#define SW_DIR			DDRD
#define PMS_OUT			PORTB
#define SW_IN			PIND
#define SW_OUT			PORTD
#define SW_BIT_BYPASS	PD0
#define SW_BIT_AB		PD1
#define LED_BIT_BYPASS	PD5
#define LED_BIT_TUNER	PD4
#define LED_BIT_A		PD3
#define LED_BIT_B		PD2
#define LED_OUT			PORTD

#define LED_BYPASS_ON()		(LED_OUT |=   (1<<LED_BIT_BYPASS))
#define LED_BYPASS_OFF()	(LED_OUT &=  ~(1<<LED_BIT_BYPASS))
#define LED_TUNER_ON()		(LED_OUT |=   (1<<LED_BIT_TUNER))
#define LED_TUNER_OFF()		(LED_OUT &=  ~(1<<LED_BIT_TUNER))
#define LED_A_ON()			(LED_OUT |=   (1<<LED_BIT_A))
#define LED_A_OFF()			(LED_OUT &=  ~(1<<LED_BIT_A))
#define LED_B_ON()			(LED_OUT |=   (1<<LED_BIT_B))
#define LED_B_OFF()			(LED_OUT &=  ~(1<<LED_BIT_B))
#define LED_ALL_OFF()		(LED_OUT &=  ~((1<<LED_BIT_BYPASS)|(1<<LED_BIT_TUNER)|(1<<LED_BIT_A)|(1<<LED_BIT_B)))


// CODE definition
// [7:0] = 

//    mode    1  2  3  4  5  6  7  8  9  10    
//  ------------------------------------------
//   bypass   1  0  0  1  0  0  1  0  0  1     
//   tuner    0  1  1  0  0  0  1  0  0  1     
//   line1    0  0  0  1  1  1  0  0  0  1     
//   line2    0  0  0  1  0  0  1  1  1  0     

//    mode    1  23  4  56  7  89  10    ENC[7:0] = { na, S23, S4, S56, S7, S89, S10, S1 }
//  ---------------------------------------------------------------------------------------
//   bypass   1  00  1  00  1  00  1                { x 0 1 0 1 0 1 1 } = 0x2B
//   tuner    0  11  0  00  1  00  1                { x 1 0 0 1 0 1 0 } = 0x4A
//   line1    0  00  1  11  0  00  1                { x 0 1 1 0 0 1 0 } = 0x32
//   line2    0  00  1  00  1  11  0                { x 0 1 0 1 1 0 0 } = 0x2C

#define	C_BYPASS	0x2B
#define	C_TUNER		0x4A
#define	C_LINEA		0x32
#define C_LINEB		0x2C


enum { S_BYPASS, S_TUNER, S_LINE };



typedef struct key_t_tag {
	uint8_t cmd;
	uint8_t stat;
	uint8_t filt;
} key_t;

/* keyin globals */
volatile static key_t key_buf;

void key_scan(void)
{
	uint8_t a, b;

	//a = (~PIND & 0x0F);
	a = (~SW_IN & ((1<<SW_BIT_AB)|(1<<SW_BIT_BYPASS)));
	if (a == key_buf.filt) {
		b = key_buf.stat;
		key_buf.stat = a;
		b = (b ^ a) & a;
		if (b) {
			key_buf.cmd = b;
		}
	}
	key_buf.filt = a;

}


enum { na = 0, btn_BYPASS, btn_LINE, btn_BYPASS_LONG };
static volatile int cmd;

void cmd_check(void)
{
	static unsigned long int check_long = 0;

	cmd = na;
 
#if 0

	if ( key_buf.stat & (1<<SW_BIT_BYPASS) ) {
		if ( check_long == 0)	check_long = 1;
		else check_long++;
	} else {
		if ( check_long ) check_long = 0;
	}

	if ( check_long > 96 ) {
		cmd = btn_BYPASS_LONG;
	} else if ( key_buf.cmd & (1<<SW_BIT_AB)) {
		cmd = btn_LINE;
		key_buf.cmd = 0;
	} else if ( key_buf.cmd & (1<<SW_BIT_BYPASS)) {
		cmd = btn_BYPASS;
		key_buf.cmd = 0;
	}

#else

	if ( key_buf.stat & (1<<SW_BIT_BYPASS) ) {
		if ( check_long == 0)	check_long = 1;
		else check_long++;
		if ( check_long > 96 ) {
			cmd = btn_BYPASS_LONG;
			key_buf.cmd = 0;
		}
	} else {
		if ( check_long ) check_long = 0;
		if ( key_buf.cmd & (1<<SW_BIT_AB)) {
			cmd = btn_LINE;
			key_buf.cmd = 0;
		} else if ( key_buf.cmd & (1<<SW_BIT_BYPASS)) {
			cmd = btn_BYPASS;
			key_buf.cmd = 0;
		}
	}

#endif

}

/*
 * Int every 2msec on 1MHz system clock
 */
ISR(TIMER0_OVF_vect)
{
	static uint8_t process;

	++process;
	// process 1 of 16 times
	if ((process & 0x0F)==0x01)	key_scan();
	if ((process & 0x0F)==0x02)	cmd_check();
	

}

void timer0Init(void)
{
	// start timer0
	TCNT0 = 0;
	TCCR0B |= (1<<CS01)|(0<<CS00);		// set prescaler by 8 1MHz/256/8 -> 2msec
	TIMSK |= (1<<TOIE0);

	sei();
}

void ioInit(void)
{
	PMS_DIR |= 0x7F;	// for Photo MOS Switch control (output)
	SW_OUT |= (1<<SW_BIT_AB);
	SW_OUT |= (1<<SW_BIT_BYPASS);
	SW_DIR &= ~(1<<SW_BIT_AB);
	SW_DIR &= ~(1<<SW_BIT_BYPASS);
	LED_DIR |= (1<<LED_BIT_BYPASS);
	LED_DIR |= (1<<LED_BIT_TUNER);
	LED_DIR |= (1<<LED_BIT_A);
	LED_DIR |= (1<<LED_BIT_B);
}

enum { LINEA = 0, LINEB = 1 };
void port_output(int state, bool tgl)
{
	static int store_LINE = LINEA;
	
	switch ( state ) {
	case S_BYPASS:
		PMS_OUT = C_BYPASS;
		LED_ALL_OFF();
		LED_BYPASS_ON();
		break;
	case S_TUNER:
		PMS_OUT = C_TUNER;
		LED_ALL_OFF();
		LED_TUNER_ON();
		break;
	case S_LINE:
		if (tgl) {
			if( store_LINE == LINEA) {
				PMS_OUT = C_LINEB;
				store_LINE = LINEB;
				LED_ALL_OFF();
				LED_B_ON();
			} else {
				PMS_OUT = C_LINEA;
				store_LINE = LINEA;
				LED_ALL_OFF();
				LED_A_ON();
			}
		} else {
			if( store_LINE == LINEA) {
				PMS_OUT = C_LINEA;
				LED_ALL_OFF();
				LED_A_ON();
			} else {
				PMS_OUT = C_LINEB;
				LED_ALL_OFF();
				LED_B_ON();
			}
		}
		break;
	}
}


int main(void)
{
	int state = S_BYPASS;
	bool tgl = false;
	
	ioInit();
	timer0Init();

	port_output(state, false);
	
    /* Replace with your application code */
    while (1) {
		sleep_cpu();
		
		if (cmd) {
			switch ( state ) {
			case S_BYPASS:
				switch ( cmd ) {
				case btn_BYPASS:
				case btn_LINE:
					state = S_LINE;
					tgl = false;
					break;
				case btn_BYPASS_LONG:
					state = S_TUNER;
					break;
				}
				cmd = na;
				break;
			case S_TUNER:
				switch ( cmd ) {
				case btn_BYPASS:
					state = S_BYPASS;
					break;
				case btn_LINE:
					state = S_LINE;
					tgl = false;
					break;
				}
				cmd = na;
				break;
			case S_LINE:
				switch ( cmd ) {
				case btn_BYPASS:
					state = S_BYPASS;
					break;		
				case btn_LINE:
					state = S_LINE;
					tgl = true;
					break;
				case btn_BYPASS_LONG:
					state = S_TUNER;
					break;
				}
				cmd = na;
				break;
			}
			port_output(state, tgl);
		}



    }
}

