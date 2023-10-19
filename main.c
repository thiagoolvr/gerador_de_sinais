

/**
 * main.c
 */

#include "F28x_Project.h"

void ConfigureGpio(void);

void Init(void) {
    ConfigureGpio();
}

int main(void) {
	InitSysCtrl();

	Init();

	while(1) {
	    GpioDataRegs.GPATOGGLE.bit.GPIO31 = 1;

	    if (GPIO_ReadPin(59) == 1)
	        GPIO_WritePin(34, true);  // LED RED OFF
	    else
	        GPIO_WritePin(34, false); // LED RED ON
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
