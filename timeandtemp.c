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

typedef enum {S_RUN, S_EDIT} state_t;
typedef enum {E_MON, E_DAY, E_HR, E_MIN, E_SEC} editstate_t;

struct tm * global_time; //Global time counter.
unsigned int timercount;

#define CALADC12_15V_30C  *((unsigned int *)0x1A1A)
#define CALADC12_15V_85C  *((unsigned int *)0x1A1C)

volatile unsigned int in_temp;
volatile unsigned int in_wheel;
volatile char temp_changed = 0;
unsigned long times[60];
float tempC[60];

char* time_str;
char* date_str;
char* temp_c_str;
char* temp_f_str;
unsigned long utc;

void swDelay(int numLoops);
int read_push_button();
void configLED1_3(char inbits);
int get_touchpad();
void set_touchpad(char states);
void swDelayShort(int numLoops);
void runtimerA2(void);
void stoptimerA2(void);
void adc_convert();

#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR(void)
{
    timercount++;
    if (timercount == 60000)
        timercount = 0;
    if (timercount % 100 == 0)
    {
        increment_tm(global_time, 1);
        adc_convert();
    }
}

#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
    in_temp = ADC12MEM0;
    in_wheel = ADC12MEM1;
    temp_changed = 1;
}

void main(void)
{
    volatile float temperatureDegC;
    volatile float temperatureDegF;
    volatile float degC_per_bit;
    volatile unsigned int bits30, bits85;
    // Stop WDT
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    REFCTL0 &= ~REFMSTR;    // Reset REFMSTR to hand over control of
    // internal reference voltages to
    // ADC12_A control registers

    P8SEL &= ~BIT0;
    P8DIR |= BIT0;
    P8OUT |= BIT0;

    ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON | ADC12MSC;     // Internal ref = 1.5V
    ADC12CTL1 = ADC12SHP | ADC12CONSEQ_1;                     // Enable sample timer
    // Using ADC12MEM0 to store reading
    ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10;  // ADC i/p ch A10 = temp sense
    ADC12MCTL1 = ADC12SREF_0 + ADC12INCH_5 + ADC12EOS;

    __delay_cycles(100);                    // delay to allow Ref to settle
    ADC12CTL0 |= ADC12ENC;                // Enable conversion
    // Use calibration data stored in info memory
    bits30 = CALADC12_15V_30C;
    bits85 = CALADC12_15V_85C;
    degC_per_bit = ((float)(85.0 - 30.0)) / ((float)(bits85 - bits30));

    ADC12IE = BIT1;
    _BIS_SR(GIE);

    //Perform initializations (see peripherals.c)
    configTouchPadLEDs();
    configDisplay();
    configCapButtons();
    global_time = (struct tm*)malloc(sizeof(struct tm));
    initialize_tm(global_time);
    GrClearDisplay(&g_sContext);

    state_t state = S_RUN;
    editstate_t editstate = E_MON;

    time_str = (char*)malloc(sizeof(char) * 9);
    date_str = (char*)malloc(sizeof(char) * 7);
    temp_c_str = (char*)malloc(sizeof(char) * 8);
    temp_f_str = (char*)malloc(sizeof(char) * 8);
    convert_time(time_str, global_time);
    convert_date(date_str, global_time);
    runtimerA2();
	volatile int btn;
	const int mon_days[] =
		  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    while (1)
    {
    	btn = read_push_button();
    	GrClearDisplay(&g_sContext);
        switch (state)
        {
		case S_RUN:
			if (btn == 1) {
				stoptimerA2();
				state = S_EDIT;
			}
			break;
		case S_EDIT:
			if (btn == 2) {
				runtimerA2();
				state = S_RUN;
				editstate = E_MON;
				mktime(global_time);
				break;
			}
			adc_convert();
			switch (editstate) {
			case E_MON:
				if (btn == 1)
					editstate = E_DAY;
				global_time->tm_mon = map(in_wheel, 0, 4085, 0, 11);
				GrLineDrawH(&g_sContext, 31, 51, 25);
				break;
			case E_DAY:
				if (btn == 1)
					editstate = E_HR;
				global_time->tm_mday = map(in_wheel, 0, 4085, 1, mon_days[global_time->tm_mon]);
				GrLineDrawH(&g_sContext, 56, 66, 25);
				break;
			case E_HR:
				if (btn == 1)
					editstate = E_MIN;
				global_time->tm_hour = map(in_wheel, 0, 4085, 0, 23);
				GrLineDrawH(&g_sContext, 27, 37, 15);
				break;
			case E_MIN:
				if (btn == 1)
					editstate = E_SEC;
				global_time->tm_min = map(in_wheel, 0, 4085, 0, 59);
				GrLineDrawH(&g_sContext, 44, 54, 15);
				break;
			case E_SEC:
				if (btn == 1)
					editstate = E_MON;
				global_time->tm_sec = map(in_wheel, 0, 4085, 0, 59);
				GrLineDrawH(&g_sContext, 62, 72, 15);
				break;
			}
			break;
		}

		if (temp_changed) {
			temperatureDegC = (float) ((long) in_temp - CALADC12_15V_30C )
					* degC_per_bit + 30.0;
			temperatureDegF = temperatureDegC * 9.0 / 5.0 + 32.0;
			temp_changed = 0;
		}
        convert_temp(temp_c_str, temperatureDegC, 0);
        convert_temp(temp_f_str, temperatureDegF, 1);
        convert_time(time_str, global_time);
        convert_date(date_str, global_time);
        utc = mktime(global_time);

        times[utc % 60] = utc;
        tempC[utc % 60] = temperatureDegC;

        GrStringDrawCentered(&g_sContext, time_str, AUTO_STRING_LENGTH, 51, 10, TRANSPARENT_TEXT);
        GrStringDrawCentered(&g_sContext, date_str, AUTO_STRING_LENGTH, 51, 20, TRANSPARENT_TEXT);
        GrStringDrawCentered(&g_sContext, temp_c_str, AUTO_STRING_LENGTH, 51, 30, TRANSPARENT_TEXT);
        GrStringDrawCentered(&g_sContext, temp_f_str, AUTO_STRING_LENGTH, 51, 40, TRANSPARENT_TEXT);
        GrFlush(&g_sContext);
    }
}

void adc_convert()
{
    ADC12CTL0 &= ~ADC12SC;  // clear the start bit
    ADC12CTL0 |= ADC12SC;       // Sampling and conversion start
}

void runtimerA2(void)
{
    TA2CTL = TASSEL_1 + MC_1 + ID_0;
    TA2CCR0 = 327;
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
    P1OUT &= ~(TOUCHPAD_1 | TOUCHPAD_2 | TOUCHPAD_3 | TOUCHPAD_4 | TOUCHPAD_5);
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
