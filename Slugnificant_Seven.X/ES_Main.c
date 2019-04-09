/*
 * File: ES_Main.c
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 * 
 * Project: Slugnificant Seven
 * Modified: Jian Hao Miao, Evan Plummer, Stephanie Lin
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/
#include <BOARD.h>
#include <xc.h>
#include <stdio.h>
#include "AD.h"
#include "pwm.h"
#include "IO_Ports.h"
#include "LED.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TapeService.h"
#include "BumperService.h"
#include "TrackService.h"
#include "TakeCoverSubHSM.h"
#include "NavigationSubSubHSM.h"

/*******************************************************************************
 * MAIN                                                                        *
 ******************************************************************************/
void main(void) {
    ES_Return_t ErrorType;

    // initialize BOARD and ports
    BOARD_Init();
    LED_Init();
    AD_Init();
    PWM_Init();
    TIMERS_Init();

    // initialize pins
    AD_AddPins(TAPE_FRONT_IN | TAPE_CENTER_IN | TAPE_BACK_IN | TAPE_LEFT_IN
            | TAPE_RIGHT_IN | TAPE_R_ANGLE_IN | TAPE_L_ANGLE_IN | TRACK_FRONT_IN 
            | TRACK_BACK_IN | BEACON_IN);

    PWM_AddPins(EN_A | EN_B | SERVO_OUT);

    IO_PortsSetPortOutputs(PORTY, TAPE_PWR_OUT | DIR_A | DIR_B | TAPE_TOGGLE);

    IO_PortsSetPortInputs(PORTZ, BUMPER_FRONT_LEFT_IN | BUMPER_FRONT_RIGHT_IN);
    IO_PortsSetPortOutputs(PORTZ, BUMPER_PWR_OUT);

    PWM_SetFrequency(PWM_10KHZ); // set PWM frequency
    PWM_SetDutyCycle(SERVO_OUT, 0); // STOP SERVO

    // initialize and start the Events and Services Framework
    ErrorType = ES_Initialize();
    if (ErrorType == Success) {
        ErrorType = ES_Run();
    }

    // error encountered if reached here
    switch (ErrorType) {
        case FailedPointer:
            printf("Failed on NULL pointer");
            break;
        case FailedInit:
            printf("Failed Initialization");
            break;
        default:
            printf("Other Failure: %d", ErrorType);
            break;
    }

    for (;;);
}
