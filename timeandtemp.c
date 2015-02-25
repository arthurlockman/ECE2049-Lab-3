#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc\hw_memmap.h"
#include "driverlibHeaders.h"
#include "CTS_Layer.h"
#include "grlib.h"
#include "LcdDriver/Dogs102x64_UC1701.h"
#include "peripherals.h"
#include "stringutils.h"

typedef enum {S_MENU, S_COUNTDOWN, S_PLAY, S_LOSE, S_WIN} state_t;

struct tm * global_time; //Global time counter.
unsigned int timercount;

void swDelay(int numLoops);
int read_push_button();
void configLED1_3(char inbits);
int get_touchpad();
void set_touchpad(char states);
void swDelayShort(int numLoops);
void runtimerA2(void);
void stoptimerA2(void);

#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR(void)
{
	increment_tm(global_time, 1);
}

void main(void)
{
	// Stop WDT
    WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

    _BIS_SR(GIE);

    //Perform initializations (see peripherals.c)
    configTouchPadLEDs();
    configDisplay();
    configCapButtons();
    initialize_tm(global_time);
    GrClearDisplay(&g_sContext);
    while(1)
    {
    }
}

void runtimerA2(void)
{
	TA2CTL = TASSEL_1 + MC_1 + ID_0;
	TA2CCR0 = 163;
	TA2CCTL0 = CCIE;
}

void stoptimerA2(void)
{
	TA2CTL = MC_0;
	TA2CCTL0 &= ~CCIE;
}

int read_push_button()
{
	P1SEL &= ~BIT7;
	P2SEL &= ~BIT2;
	P1DIR &= ~BIT7;
	P2DIR &= ~BIT2;
	P1REN |= BIT7;
	P2REN |= BIT2;
	P1OUT &= BIT7;
	P2OUT &= BIT2;
	if ((P1IN & BIT7) == 0 && (P2IN & BIT2) == 0) return 3;
	else if ((P1IN & BIT7) == 0) return 1;
	else if ((P2IN & BIT2) == 0) return 2;
	else return 0;
}

void configLED1_3(char inbits)
{
    P1SEL &= ~BIT0;
    P8SEL &= ~(BIT1 | BIT2);
    P1DIR |= BIT0;
    P8DIR |= (BIT1 | BIT2);
    P1OUT &= ~BIT0;
    P8OUT &= ~(BIT1 | BIT2);
    if ((inbits & BIT0) != 0) P1OUT |= BIT0;
    if ((inbits & BIT1) != 0) P8OUT |= BIT1;
    if ((inbits & BIT2) != 0) P8OUT |= BIT2;
}

void set_touchpad(char states)
{
	P1OUT &= ~(TOUCHPAD_1|TOUCHPAD_2|TOUCHPAD_3|TOUCHPAD_4|TOUCHPAD_5);
	P1OUT |= (states);
}

int get_touchpad()
{
    //Get touchpad button pressed. Returns column that the button corresponds to.
	switch (CapButtonRead())
	{
	case BUTTON_NONE:
		return -1;
	case BUTTON_X:
		return TOUCHPAD_1;
	case BUTTON_SQ:
		return TOUCHPAD_2;
	case BUTTON_OCT:
		return TOUCHPAD_3;
	case BUTTON_TRI:
		return TOUCHPAD_4;
	case BUTTON_CIR:
		return TOUCHPAD_5;
	default:
		return -1;
	}
}

/**
 * @brief Delay a specified number of loops/
 *
 * @param numLoops The number of loops to delay.
 */
void swDelay(int numLoops)
{
    volatile unsigned long int i, j; // volatile to prevent optimization
    // by compiler

    for (j = 0; j < numLoops; j++)
    {
        i = 50000;                   // SW Delay
        while (i > 0)               // could also have used while (i)
            i--;
    }
}

void swDelayShort(int numLoops)
{
    volatile unsigned long int i, j; // volatile to prevent optimization
    // by compiler

    for (j = 0; j < numLoops; j++)
    {
        i = 500;                   // SW Delay
        while (i > 0)               // could also have used while (i)
            i--;
    }
}
