/**********************************************************************/
// ENGR-2350 Final Project
// Name: Kendall Gordinier
// RIN: 662020130
// Name: Jamie Draper
// RIN: 662024490
/**********************************************************************/


#include "engr2350_msp432.h"

void Timer_Init();
void GPIO_Init();
void Encoder_ISR();
void Timer_ISR();
void T1_100ms_ISR();
void Diode();


Timer_A_UpModeConfig timerconfig;
uint8_t time[4];
uint8_t toggle = 0;
uint16_t timer_reset_count = 1;

Timer_A_UpModeConfig TA0cfg;
Timer_A_UpModeConfig TA1cfg;
Timer_A_ContinuousModeConfig TA3cfg;
Timer_A_CompareModeConfig TA0_ccr3;
Timer_A_CompareModeConfig TA0_ccr4;
Timer_A_CaptureModeConfig TA3_ccr0;
Timer_A_CaptureModeConfig TA3_ccr1;

Timer_A_ContinuousModeConfig TA2cfg;
Timer_A_CaptureModeConfig TA2_ccr3;
Timer_A_CaptureModeConfig TA2_ccr2;



// Encoder total events
uint32_t enc_total_L,enc_total_R;
// Speed measurement variables
int32_t Tach_L_count,Tach_L,Tach_L_sum,Tach_L_sum_count,Tach_L_avg; // Left wheel
int32_t Tach_R_count,Tach_R,Tach_R_sum,Tach_R_sum_count,Tach_R_avg; // Right wheel

int32_t diode_counter;

int32_t timercount;
int32_t fallingedgepos;
float duty;

uint8_t run_control = 0; // Flag to denote that 100ms has passed and control should be run.

float ki = 1;
float error_sum_l = 0;
float error_sum_r = 0;

float targetspeed = 50;
float diffspeed;

float target_l = 0;
float target_r = 0;

float actual = 0;


int16_t pwm_max = 720;
int16_t pwm_min = 20;
int16_t pwm_set_l = 0;
int16_t pwm_set_r = 0;

int16_t rising = 0;
int16_t falling = 0;


int main(void) /* Main Function */
{

    SysInit();

    Timer_Init();
    GPIO_Init();

    __delay_cycles(24e6);

    GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN6|GPIO_PIN7);   // Motors are ON

    while(1){
        if(run_control){    // If 100 ms has passed
            run_control = 0;    // Reset the 100 ms flag
            printf("%f\r\n",duty);
            if (diffspeed < -100){
                target_l = 0;
                target_r = 0;
            }else{
                target_l = targetspeed + diffspeed;
                target_r = targetspeed - diffspeed;
            }
            printf("speed: %f\r\n", diffspeed);

            error_sum_r += target_r-15000*800/Tach_R_avg; // perform "integration"
            pwm_set_r = target_r + ki*error_sum_r;

            if(pwm_set_r > pwm_max) pwm_set_r = pwm_max;  // Set limits on the pwm control output
            if(pwm_set_r < pwm_min) pwm_set_r = pwm_min;
            Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_3,pwm_set_r);


            error_sum_l += target_l-15000*800/Tach_L_avg; // perform "integration"

            pwm_set_l = target_l + ki*error_sum_l;

            if(pwm_set_l > pwm_max) pwm_set_l = pwm_max;  // Set limits on the pwm control output
            if(pwm_set_l < pwm_min) pwm_set_l = pwm_min;
            Timer_A_setCompareValue(TIMER_A0_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_4,pwm_set_l);


        }
    }
}

void Timer_Init(){
    timerconfig.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timerconfig.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    timerconfig.timerPeriod = ((24000000/64)*(0.1));
    timerconfig.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_registerInterrupt( TIMER_A0_BASE , TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT , Timer_ISR );
    Timer_A_configureUpMode( TIMER_A0_BASE, & timerconfig);
    Timer_A_startCounter( TIMER_A0_BASE , TIMER_A_UP_MODE);

    // Configure PWM timer for 30 kHz
        TA0cfg.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
        TA0cfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
        TA0cfg.timerPeriod = 800;
        Timer_A_configureUpMode(TIMER_A0_BASE,&TA0cfg);
        // Configure TA0.CCR3 for PWM output
        TA0_ccr3.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
        TA0_ccr3.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
        TA0_ccr3.compareValue = 0;
        Timer_A_initCompare(TIMER_A0_BASE,&TA0_ccr3);
        // Configure TA0.CCR4 for PWM output
        TA0_ccr4.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_4;
        TA0_ccr4.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
        TA0_ccr4.compareValue = 0;
        Timer_A_initCompare(TIMER_A0_BASE,&TA0_ccr4);
        // Configure Encoder timer in continuous mode
        TA3cfg.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
        TA3cfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
        TA3cfg.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
        Timer_A_configureContinuousMode(TIMER_A3_BASE,&TA3cfg);
        // Configure TA3.CCR0 for Encoder measurement
        TA3_ccr0.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_0;
        TA3_ccr0.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
        TA3_ccr0.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
        TA3_ccr0.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
        TA3_ccr0.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
        Timer_A_initCapture(TIMER_A3_BASE,&TA3_ccr0);
        // Configure TA3.CCR1 for Encoder measurement
        TA3_ccr1.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
        TA3_ccr1.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
        TA3_ccr1.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
        TA3_ccr1.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
        TA3_ccr1.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
        Timer_A_initCapture(TIMER_A3_BASE,&TA3_ccr1);
        // Register the Encoder interrupt
        Timer_A_registerInterrupt(TIMER_A3_BASE,TIMER_A_CCR0_INTERRUPT,Encoder_ISR);
        Timer_A_registerInterrupt(TIMER_A3_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,Encoder_ISR);
        // Configure 10 Hz timer
        TA1cfg.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
        TA1cfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
        TA1cfg.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
        TA1cfg.timerPeriod = 37500;
        Timer_A_configureUpMode(TIMER_A1_BASE,&TA1cfg);
        Timer_A_registerInterrupt(TIMER_A1_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,T1_100ms_ISR);
        // Start all the timers
        Timer_A_startCounter(TIMER_A0_BASE,TIMER_A_UP_MODE);
        Timer_A_startCounter(TIMER_A1_BASE,TIMER_A_UP_MODE);
        Timer_A_startCounter(TIMER_A3_BASE,TIMER_A_CONTINUOUS_MODE);
        // Configure Encoder timer in continuous mode
        TA2cfg.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
        TA2cfg.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
        TA2cfg.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
        Timer_A_configureContinuousMode(TIMER_A2_BASE,&TA2cfg);

        // Configure TA3.CCR0 for Encoder measurement
        TA2_ccr3.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_3;
        TA2_ccr3.captureMode = TIMER_A_CAPTUREMODE_RISING_EDGE;
        TA2_ccr3.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
        TA2_ccr3.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
        TA2_ccr3.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
        TA2_ccr2.captureRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;
        TA2_ccr2.captureMode = TIMER_A_CAPTUREMODE_FALLING_EDGE;
        TA2_ccr2.captureInputSelect = TIMER_A_CAPTURE_INPUTSELECT_CCIxA;
        TA2_ccr2.synchronizeCaptureSource = TIMER_A_CAPTURE_SYNCHRONOUS;
        TA2_ccr2.captureInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
        Timer_A_initCapture(TIMER_A2_BASE,&TA2_ccr3);
        Timer_A_initCapture(TIMER_A2_BASE,&TA2_ccr2);
        Timer_A_registerInterrupt(TIMER_A2_BASE,TIMER_A_CCR0_INTERRUPT,Diode);
        Timer_A_registerInterrupt(TIMER_A2_BASE,TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT,Diode);
        Timer_A_startCounter(TIMER_A2_BASE, TIMER_A_CONTINUOUS_MODE);


}

void GPIO_Init(){

    GPIO_setAsOutputPin(GPIO_PORT_P5,GPIO_PIN4|GPIO_PIN5);   // Motor direction pins
    GPIO_setAsOutputPin(GPIO_PORT_P3,GPIO_PIN6|GPIO_PIN7);   // Motor enable pins
    // Motor PWM pins
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2,GPIO_PIN6|GPIO_PIN7,GPIO_PRIMARY_MODULE_FUNCTION);
    // Motor Encoder pins
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P10,GPIO_PIN4|GPIO_PIN5,GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5,GPIO_PIN4|GPIO_PIN5);   // Motors set to forward
    GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN6|GPIO_PIN7);   // Motors are OFF
}


void Encoder_ISR(){
    // If encoder timer has overflowed...
    if(Timer_A_getEnabledInterruptStatus(TIMER_A3_BASE) == TIMER_A_INTERRUPT_PENDING){
        Timer_A_clearInterruptFlag(TIMER_A3_BASE);
        Tach_L_count += 65536;
        Tach_R_count += 65536;
    // Otherwise if the Left Encoder triggered...
    }else if(Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1)&TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG){
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);
        enc_total_L++;   // Increment the total number of encoder events for the left encoder
        // Calculate and track the encoder count values
        Tach_L = Tach_L_count + Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);
        Tach_L_count = -Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);
        // Sum values for averaging
        Tach_L_sum_count++;
        Tach_L_sum += Tach_L;
        // If 6 values have been received, average them.
        if(Tach_L_sum_count == 6){
            Tach_L_avg = Tach_L_sum/6;
            Tach_L_sum_count = 0;
            Tach_L_sum = 0;
        }
    // Otherwise if the Right Encoder triggered...
    }else if(Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0)&TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG){
        Timer_A_clearCaptureCompareInterrupt(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
        enc_total_R++;
        Tach_R = Tach_R_count + Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
        Tach_R_count = -Timer_A_getCaptureCompareCount(TIMER_A3_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
        Tach_R_sum_count++;
        Tach_R_sum += Tach_R;
        if(Tach_R_sum_count == 6){
            Tach_R_avg = Tach_R_sum/6;
            Tach_R_sum_count = 0;
            Tach_R_sum = 0;


        }
    }
}

void Timer_ISR(){
    Timer_A_clearInterruptFlag( TIMER_A0_BASE );
    timer_reset_count++;

}
void T1_100ms_ISR(){
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);
    run_control = 1;
}

void Diode(){
    // If encoder timer has overflowed...
        if(Timer_A_getEnabledInterruptStatus(TIMER_A2_BASE) == TIMER_A_INTERRUPT_PENDING){
            Timer_A_clearInterruptFlag(TIMER_A2_BASE);
            timercount += 65536;

        // Otherwise if the Left Encoder triggered...
        }else if(Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A2_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_3)&TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG){
            Timer_A_clearCaptureCompareInterrupt(TIMER_A2_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_3);

            duty = (fallingedgepos) / ((float)timercount + (float)Timer_A_getCaptureCompareCount(TIMER_A2_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_3));
            diffspeed = (duty*800 - 450)/ 400 * 12.5;
            timercount = -Timer_A_getCaptureCompareCount(TIMER_A2_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_3);


        }else if(Timer_A_getCaptureCompareEnabledInterruptStatus(TIMER_A2_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_2)&TIMER_A_CAPTURECOMPARE_INTERRUPT_FLAG){
            Timer_A_clearCaptureCompareInterrupt(TIMER_A2_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_2);
            fallingedgepos = timercount + Timer_A_getCaptureCompareCount(TIMER_A2_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_2);

        }

}
