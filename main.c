

/**
 * main.c
 */

#include "F28x_Project.h"
#include "math.h"

#define SAMPLE_FREQ     10000.f // Timer interruption frequency
#define SIGNAL_FREQ     100.f
#define CPU_FREQ        200000000.f
#define ADC_BUFFER_SIZE 256
#define DAC_FULL_SCALE  4096
#define DAC_MAX_VALUE   DAC_FULL_SCALE - 48
#define DAC_MIN_VALUE   48
#define PI              3.14159265359f

void cpu_timer0_isr(void);
void ConfigureGpio(void);
void ConfigureDac(void);
void ConfigureAdc(void);
void ConfigureTimer(void);

Uint16 sineWave(float time);
Uint16 squareWave(float time, float signal_period);
Uint16 triangleWave(float time, float signal_period);

void Init(void) {
    ConfigureGpio();
    ConfigureDac();
    ConfigureAdc();
    ConfigureTimer();
}

void TimerCallback(void) {
    static float time = 0;
    static float delta_time = 1.f/SAMPLE_FREQ;
    static const float signal_period = 1.f/SIGNAL_FREQ;

    GpioDataRegs.GPATOGGLE.bit.GPIO31 = 1;
    DacaRegs.DACVALS.all = sineWave(time);
    DacbRegs.DACVALS.all = triangleWave(time, signal_period);

    time += delta_time;
    if(time > signal_period)
        time = 0;
}

int main(void) {
	InitSysCtrl();

	Init();

	while(1) {

	}
}

void ConfigureGpio(void) {
    InitGpio();
    EALLOW;
    GPIO_SetupPinOptions(31, GPIO_OUTPUT, 0);
    GPIO_SetupPinOptions(34, GPIO_OUTPUT, 0);
    GPIO_SetupPinOptions(59, GPIO_INPUT, GPIO_PULLUP);
    EDIS;
}

void ConfigureTimer(void) {
    DINT;

    InitPieCtrl();
    IER = 0;
    IFR = 0;
    InitPieVectTable();

    //ConfigCpuTimer(&CpuTimer0, 200, 1'000'000);

    // CPU frequency * interruption period (in seconds)
    CpuTimer0Regs.PRD.all = (uint32_t)(CPU_FREQ/SAMPLE_FREQ);
    CpuTimer0Regs.TCR.bit.TSS  = 1; // 1 = Stop timer, 0 = Start/Restart Timer
    CpuTimer0Regs.TCR.bit.TRB  = 1; // 1 = reload timer
    CpuTimer0Regs.TCR.bit.SOFT = 0;
    CpuTimer0Regs.TCR.bit.FREE = 0; // Timer Free Run Disabled
    CpuTimer0Regs.TCR.bit.TIE  = 1; // 0 = Disable/ 1 = Enable Timer Interrupt
    CpuTimer0Regs.TCR.bit.TSS  = 0; // 1 = Stop timer, 0 = Start/Restart Timer

    EALLOW;
    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
    EDIS;

    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    IER |= M_INT1;

    EINT;
    ERTM;
}

void ConfigureAdc(void) {
    EALLOW;
    // Set ADCCLK divider to /4
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    //  Power up the ADC
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    // ADCINA0
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;
    // sample duration of 20 SYSCLK cycles
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 19;
    // Timer 0
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 1;
    EDIS;
}

void ConfigureDac(void) {
    EALLOW;
    // Use adc references
    DacaRegs.DACCTL.bit.DACREFSEL = 1;
    // Enable DAC
    DacaRegs.DACOUTEN.bit.DACOUTEN = 1;
    // Use adc references
    DacbRegs.DACCTL.bit.DACREFSEL = 1;
    // Enable DAC
    DacbRegs.DACOUTEN.bit.DACOUTEN = 1;
    EDIS;
}

__interrupt void cpu_timer0_isr(void) {
    TimerCallback();
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

Uint16 squareWave(float time, float signal_period) {
    if(time <= signal_period/2)
        return DAC_MIN_VALUE;
    else if(time <= signal_period)
        return DAC_MAX_VALUE;
    else
        return 0; // ERROR CASE
}

Uint16 triangleWave(float time, float signal_period) {
    static const float slope = (DAC_MAX_VALUE - DAC_MIN_VALUE)*SIGNAL_FREQ*2;

    if(time <= signal_period/2)
        return (Uint16)(DAC_MIN_VALUE + slope*time);
    else if (time <= signal_period)
        return (Uint16)(DAC_MAX_VALUE - slope*(time-(signal_period/2)));
    else
        return 0; // ERROR CASE
}

Uint16 sineWave(float time) {
    static const float amplitude = (DAC_MAX_VALUE - DAC_MIN_VALUE)/2,
                       offset = DAC_FULL_SCALE/2,
                       w = 2*PI*SIGNAL_FREQ;

    return (Uint16)(amplitude*sin(w*time) + offset);
}
