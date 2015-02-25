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

unsigned int in_temp;
char temp_changed;

void swDelay(int numLoops);
int read_push_button();
void configLED1_3(char inbits);
int get_touchpad();
void set_touchpad(char states);
void swDelayShort(int numLoops);
void runtimerA2(void);
void stoptimerA2(void);
void convert_temps();

#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR(void)
{
    timercount++;
    if (timercount == 60000)
        timercount = 0;
    if (timercount % 100 == 0)
    {
        increment_tm(global_time, 1);
        convert_temps();
    }
}

#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
    in_temp = ADC12MEM0;
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

    ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON;     // Internal ref = 1.5V
    ADC12CTL1 = ADC12SHP;                     // Enable sample timer
    // Using ADC12MEM0 to store reading
    ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10;  // ADC i/p ch A10 = temp sense
    // ACD12SREF_1 = internal ref = 1.5v

    __delay_cycles(100);                    // delay to allow Ref to settle
    ADC12CTL0 |= ADC12ENC;                // Enable conversion
    // Use calibration data stored in info memory
    bits30 = CALADC12_15V_30C;
    bits85 = CALADC12_15V_85C;
    degC_per_bit = ((float)(85.0 - 30.0)) / ((float)(bits85 - bits30));

    ADC12IE = BIT0;
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

    char* time_str = (char*)malloc(sizeof(char) * 9);
    char* date_str = (char*)malloc(sizeof(char) * 7);
    char* temp_c_str = (char*)malloc(sizeof(char) * 8);
    char* temp_f_str = (char*)malloc(sizeof(char) * 8);
    convert_time(time_str, global_time);
    convert_date(date_str, global_time);
    runtimerA2();
    while (1)
    {
        switch (state)
        {
        case S_RUN:
            convert_time(time_str, global_time);
            convert_date(date_str, global_time);
            break;
        case S_EDIT:
            break;
        }

        if (temp_changed)
        {
            temperatureDegC = (float)((long)in_temp - CALADC12_15V_30C) * degC_per_bit + 30.0;
            temperatureDegF = temperatureDegC * 9.0 / 5.0 + 32.0;
            temp_changed = 0;
        }
        GrClearDisplay(&g_sContext);
        convert_temp(temp_c_str, temperatureDegC, 0);
        convert_temp(temp_f_str, temperatureDegF, 1);
        GrStringDrawCentered(&g_sContext, time_str, AUTO_STRING_LENGTH, 51, 10, TRANSPARENT_TEXT);
        GrStringDrawCentered(&g_sContext, date_str, AUTO_STRING_LENGTH, 51, 20, TRANSPARENT_TEXT);
        GrStringDrawCentered(&g_sContext, temp_c_str, AUTO_STRING_LENGTH, 51, 30, TRANSPARENT_TEXT);
        GrStringDrawCentered(&g_sContext, temp_f_str, AUTO_STRING_LENGTH, 51, 40, TRANSPARENT_TEXT);
        GrFlush(&g_sContext);
        __no_operation();                       // SET BREAKPOINT HERE
    }
}

void convert_temps()
{
    ADC12CTL0 &= ~ADC12SC;  // clear the start bit
    ADC12CTL0 |= ADC12SC;       // Sampling and conversion start
    // Single conversion (single channel)
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
