/*
 * File: TakeCoverSubHSM.h
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 * 
 * Project: Slugnificant Seven
 * Modified: Jian Hao Miao, Evan Plummer, Stephanie Lin
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/
#include <stdio.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "LED.h"
#include "IO_Ports.h"
#include "TapeService.h"
#include "BeaconService.h"
#include "HSM.h"
#include "TakeCoverSubHSM.h"
#include "NavigationSubSubHSM.h"
#include "pwm.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define SWIVEL_TIME            950 // swivel delays

typedef enum {
    InitPSubState,
    Position1,
    Position2,
    Position3,
    Offset,
    Swivel,
    Swive2,
    LocateEnemy,
    Fire
} IFZSubHSMState_t;

static const char *StateNames[] = {
	"InitPSubState",
	"Position1",
	"Position2",
	"Position3",
	"Offset",
	"Swivel",
	"Swive2",
	"LocateEnemy",
};

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
ES_Event SendDriveCommand2(uint8_t command, uint8_t mode);

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
static IFZSubHSMState_t CurrentState = InitPSubState;
static uint8_t MyPriority;

static uint8_t NumRightAngleTurns;
static uint8_t SpinSequence;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/
uint8_t InitTakeCoverSubHSM(void) {
    ES_Event returnEvent;

    // post initial transition
    CurrentState = InitPSubState;
    returnEvent = RunTakeCoverSubHSM(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

ES_Event RunTakeCoverSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE;
    IFZSubHSMState_t nextState;

    ES_Event passEvent;
    uint8_t correctionSequence, stopSwivel, altSequence;
    static uint8_t toggle;

    //ES_Tattle(); ?

    switch (CurrentState) {
        case InitPSubState:
            if (ThisEvent.EventType == ES_INIT) {
                nextState = Position1; // STARTING STATE
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        case Position1:
            switch (ThisEvent.EventType) {
                case START_BOT:
                    printf("\r\n POSITION 1");
                    SendDriveCommand2(SPIN_RIGHT, 2);
                    break;
                case BEACON_EVENT:
                    ThisEvent = RunNavigationSubSubHSM(ThisEvent);
                    nextState = Position2;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                default:
                    break;
            }
            break;
        case Position2: // GET ONTO TAPE (2)
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("\r\n POSITION 2");
                    PWM_SetDutyCycle(SERVO_OUT, 800);
                    SendDriveCommand2(FORWARD_TIMED_MEDIUM, 1);
                    break;
                case ES_TIMEOUT:
                    ThisEvent = RunNavigationSubSubHSM(ThisEvent);
                    nextState = Swivel;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                default:
                    break;
            }
            break;
        case Position3:
            switch(ThisEvent.EventType) {
                case ES_ENTRY:
                    SendDriveCommand2(FORWARD_TIMED_MEDIUM, 1);
                    break;
            }
//        case Position3: // POSITION ON TAPE
//            switch (ThisEvent.EventType) {
//                case ES_ENTRY:
//                    PWM_SetDutyCycle(SERVO_OUT, 800);
//                    SendDriveCommand2(FORWARD, 2);
//                    break;
//                case TAPE_EVENT:
//                    if ((ThisEvent.EventParam & BACK_TAPE) == BACK_TAPE) {
//                        SendDriveCommand2(STOP, 1);
//                        nextState = LocateEnemy;
//                        makeTransition = TRUE;
//                        ThisEvent.EventType = ES_NO_EVENT;
//                    } else if ((ThisEvent.EventParam & CENTER_TAPE) == CENTER_TAPE) {
//                        SendDriveCommand2(SPIN_RIGHT, 1);
//                    }
//                    break;
//            }
//            break;
//        case LocateEnemy:
//            switch (ThisEvent.EventType) {
//                case ES_ENTRY:
//                    SendDriveCommand2(SPIN_RIGHT, 2);
//                    break;
//                case BEACON_EVENT:
//                    if (ThisEvent.EventParam == DETECTED) {
//                        PWM_SetDutyCycle(SERVO_OUT, 800);
//                        SendDriveCommand2(STOP, 1);
//                        nextState = Offset;
//                        makeTransition = TRUE;
//                        ThisEvent.EventType = ES_NO_EVENT;
//                    }
//                    break;
//                default:
//                    break;
//            }
//            break;
        case Offset:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    SendDriveCommand2(SPIN_RIGHT, 2);
                    ES_Timer_InitTimer(HSM_TIMER_2, 150);
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == HSM_TIMER_2) {
                        SendDriveCommand2(STOP, 1);
                        nextState = Swivel;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        break;
                    }
                default:
                    break;
            }
            break;
//        case Swivel:
//            switch (ThisEvent.EventType) {
//                case ES_ENTRY:
//                    if (toggle == 0) {
//                        // printf("\r\n SWIVEL 1");
//                        SendDriveCommand2(SPIN_LEFT, 2);
//                        ES_Timer_InitTimer(HSM_TIMER_2, 300);
//                    } else {
//                        // printf("\r\n SWIVEL 2");
//                        SendDriveCommand2(SPIN_RIGHT, 2);
//                        ES_Timer_InitTimer(HSM_TIMER_2, 300);
//                    }
//                    break;
//                case ES_TIMEOUT:
//                    toggle ^= 1;
//                    nextState = Swivel;
//                    makeTransition = TRUE;
//                    ThisEvent.EventType = ES_NO_EVENT;
//                    break;
//            }
//            break;
        default:
            break;
    } // end switch on Current State

    if (makeTransition == TRUE) {
        RunTakeCoverSubHSM(EXIT_EVENT);
        CurrentState = nextState;
        RunTakeCoverSubHSM(ENTRY_EVENT);
    }

    ES_Tail();

    return ThisEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
ES_Event SendDriveCommand2(uint8_t command, uint8_t mode) {
    // pass command to subsubHSM
    ES_Event driveCommand;

    // mode = 1 regular speed, mode = 0 slowed by 200
    if (mode == 1) {
        driveCommand.EventType = DRIVE_EVENT_1;
    } else {
        driveCommand.EventType = DRIVE_EVENT_2;
    }

    driveCommand.EventParam = command;
    ES_Event returnEvent = RunNavigationSubSubHSM(driveCommand);
    return returnEvent;
}
