/**********************************************************************/
// ENGR-2350 Template Project
// Name: Jamie Draper
// RIN: 662024490
// Name2: Kendall Gordinier
// RIN2: 662020130
// This is the base project for several activities and labs throughout
// the course.  The outline provided below isn't necessarily *required*
// by a C program; however, this format is required within ENGR-2350
// to ease debugging/grading by the staff.
/**********************************************************************/

// We'll always add this include statement. This basically takes the
// code contained within the "engr_2350_msp432.h" file and adds it here.
#include "engr2350_msp432.h"

// Add function prototypes here, as needed.
void GPIO_Init();
void Timer_Init();
void T1_100ms_ISR();
void ADCInit();

// Add global variables here, as needed.

Timer_A_UpModeConfig config;
Timer_A_UpModeConfig TA1cfg;

Timer_A_CompareModeConfig compCfg1;

uint16_t direction;

uint8_t run_control;



int main(void) /* Main Function */
{
    // Add local variables here, as needed.

    // We always call the "SysInit()" first to set up the microcontroller
    // for how we are going to use it.
    SysInit();
    // Place initialization code (or run-once) code here

    GPIO_Init();
    Timer_Init();
    ADCInit();


    while(1){
        // Place code that runs continuously in here
        if(run_control){    // If 100 ms has passed
            run_control = 0;
            ADC14_toggleConversionTrigger();

            while(ADC14_isBusy()){

            }
            direction = ADC14_getResult(ADC_MEM0) * 749 / 16384 + 50;
            printf("%u\r\n",direction);

            Timer_A_setCompareValue(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_3, direction);
        }


    }
}

void GPIO_Init(){


    GPIO_setAsPeripheralModuleFunctionOutputPin(2, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);
}

void Timer_Init(){
    config.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    config.timerPeriod = 800;

    compCfg1.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
    compCfg1.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    compCfg1.compareValue = 0;
    Timer_A_initCapture(TIMER_A0_BASE, &compCfg1);



    Timer_A_configureUpMode(TIMER_A0_BASE, &config);


    Timer_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);


    // Configure 10 Hz timer
       TA1cfg.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
       TA1cfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
       TA1cfg.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
       TA1cfg.timerPeriod = 37500;
       Timer_A_configureUpMode(TIMER_A1_BASE,&TA1cfg);
       Timer_A_registerInterrupt(TIMER_A1_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,T1_100ms_ISR);
       // Start all the timers
       Timer_A_startCounter(TIMER_A1_BASE,TIMER_A_UP_MODE);
}

void T1_100ms_ISR(){
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    run_control = 1;
}

void ADCInit(){
    // Add your ADC initialization code here.
    //  Don't forget the GPIO, either here or in GPIOInit()!!
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_SMCLK,1,4, 0);
    ADC14_setResolution(ADC_14BIT);

    ADC14_configureConversionMemory(ADC_MEM0,ADC_VREFPOS_AVCC_VREFNEG_VSS,ADC_INPUT_A12,false);

    //pin4.1

    ADC14_configureSingleSampleMode(ADC_MEM0,true);
    //ADC14_configureConversionMemory(ADC_MEM0,ADC_VREFPOS_AVCC_VREFNEG_VSS,ADC_INPUT_A15,false);
    //ADC14_configureSingleSampleMode(ADC_MEM0,true);

    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);
    ADC14_enableConversion();
}
// Add function declarations here as needed

// Add interrupt functions last so they are easy to find
