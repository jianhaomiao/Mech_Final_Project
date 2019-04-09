/*
 * File: IFZSubHSM.h
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
#include "IFZSubHSM.h"
#include "NavigationSubSubHSM.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

#define RIGHT 0
#define LEFT 1

typedef enum {
    InitPSubState,
    Orient,
    Center,
    TapeFollow,
    Evade,
    IFZ,
    Start,
} IFZSubHSMState_t;

static const char *StateNames[] = {
	"InitPSubState",
	"Orient",
	"Center",
	"TapeFollow",
	"Evade",
	"IFZ",
	"Start",
};

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
ES_Event SendDriveCommand1(uint8_t command, uint8_t mode);
void DrivePotorSpeeds(int dutyCycleA, int dutyCycleB);

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
static IFZSubHSMState_t CurrentState = InitPSubState;
static uint8_t MyPriority, Tracker;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/
uint8_t InitIFZSubHSM(void) {
    ES_Event returnEvent;

    // post initial transition
    CurrentState = InitPSubState;
    returnEvent = RunIFZSubHSM(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

ES_Event RunIFZSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE;
    IFZSubHSMState_t nextState;

    static uint8_t correctionSequence, numRightAngleTurns, timeoutCounter;
    static uint8_t driveSequence, startCounter, alignTape, altSequence, startTimer;
    static uint16_t initTimerTrigger, initTimerTrigger2;
    static uint8_t cornerTurn;

    ES_Tattle();

    switch (CurrentState) {
        case InitPSubState:
            if (ThisEvent.EventType == BUMPER_EVENT) {
                nextState = Orient; // INITIAL STATE
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        case Orient: // SPIN UNTIL BEACON DETECTED
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    SendDriveCommand1(SPIN_RIGHT, 1);
                    break;
                case BEACON_EVENT:
                    if (ThisEvent.EventParam == DETECTED) {
                        SendDriveCommand1(STOP, 1); // STOP ROTATION            
                        nextState = Center;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_TIMEOUT:
                    RunNavigationSubSubHSM(ThisEvent);
                    break;
                default:
                    break;
            }
            break;
        case Center: // GET ONTO TAPE
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("\r\n === CENTER ===");
                    SendDriveCommand1(FORWARD, 2);
                    break;
                case TAPE_EVENT:
                    if (alignTape == RIGHT) { // GO RIGHT
                        if ((ThisEvent.EventParam & BACK_TAPE) == BACK_TAPE) {
                            SendDriveCommand1(STOP, 1);
                            nextState = TapeFollow;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        } else if ((ThisEvent.EventParam & CENTER_TAPE) == CENTER_TAPE) {
                            SendDriveCommand1(SPIN_RIGHT, 1);
                        } else if ((ThisEvent.EventParam & RIGHT_TAPE) == RIGHT_TAPE) {
                            SendDriveCommand1(SPIN_RIGHT, 1);
                        } else if ((ThisEvent.EventParam & FRONT_TAPE) == FRONT_TAPE) {
                            SendDriveCommand1(PIVOT_FORWARD_RIGHT, 1);
                        }
                    } else { // GO LEFT
                        if ((ThisEvent.EventParam & BACK_TAPE) == BACK_TAPE) {
                            SendDriveCommand1(STOP, 1);
                            nextState = TapeFollow;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                        } else if ((ThisEvent.EventParam & CENTER_TAPE) == CENTER_TAPE) {
                            SendDriveCommand1(SPIN_LEFT, 1);
                        } else if ((ThisEvent.EventParam & LEFT_TAPE) == LEFT_TAPE) {
                            SendDriveCommand1(SPIN_LEFT, 1);
                        } else if ((ThisEvent.EventParam & FRONT_TAPE) == FRONT_TAPE) {
                            SendDriveCommand1(PIVOT_FORWARD_LEFT, 1);
                        }
                    }
                default:
                    break;
            }
            break;
        case Evade: // DRIVE INTO OBSTACLE AND BACK UP
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    // printf("\r\n === EVADE ===");
                    SendDriveCommand1(FORWARD_TIMED_MEDIUM, 1);
                    driveSequence = 0;
                    break;
                case ES_TIMEOUT:
                    RunNavigationSubSubHSM(ThisEvent);
                    switch (driveSequence) {
                        case 0:
                            // printf("\r\n CASE 0");
                            SendDriveCommand1(BACKWARDS_TIMED_SHORT, 1);
                            driveSequence += 1;
                            break;
                        case 1:
                            // printf("\r\n CASE 1");
                            if (alignTape == RIGHT) {
                                SendDriveCommand1(HARD_TURN_90_RIGHT, 1);
                            } else {
                                SendDriveCommand1(HARD_TURN_90_LEFT, 1);
                            }
                            driveSequence += 1;
                            break;
                        case 2:
                            // printf("\r\n CASE 2");
                            SendDriveCommand1(FORWARD_TIMED_LONG, 1);
                            driveSequence += 1;
                            break;
                        case 3:
                            // printf("\r\n CASE 3");
                            if (alignTape == RIGHT) {
                                alignTape = LEFT;
                            } else {
                                alignTape = RIGHT;
                            }
                            nextState = Center;
                            makeTransition = TRUE;
                            ThisEvent.EventType = ES_NO_EVENT;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case TapeFollow: // FOLLOW THE TAPE AND GET TO IFZ
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("\r\n === TAPE FOLLOW ===");
                    if (alignTape == RIGHT) {
                        printf("\r\n == ALIGN RIGHT ==");
                    } else {
                        printf("\r\n == ALIGN LEFT ==");
                    }
                    SendDriveCommand1(FORWARD, 1);
                    startTimer = FALSE;
                    ES_Timer_InitTimer(HSM_TIMER_2, 500);
                    break;
                case BUMPER_EVENT:
                    nextState = Evade;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case TAPE_EVENT:
                    // KEEP TRACK OF NUMBER OF 90 DEGREE TURNS
                    if (alignTape == RIGHT) {
                        if (((ThisEvent.EventParam & RIGHT_ANGLE_TAPE) == RIGHT_ANGLE_TAPE) && (startCounter == TRUE)) {
                            if (ThisEvent.EventParam & (FRONT_TAPE | CENTER_TAPE | LEFT_TAPE) != 0) {
                                if ((TIMERS_GetTime() - initTimerTrigger) > 5000) {
                                    printf("\r\n RIGHT ANGLE: %d", numRightAngleTurns);
                                    initTimerTrigger = TIMERS_GetTime();
                                    numRightAngleTurns += 1;
                                }
                            }
                        }
                    } else {
                        if (((ThisEvent.EventParam & LEFT_ANGLE_TAPE) == LEFT_ANGLE_TAPE) && (startCounter == TRUE)) {
                            if (ThisEvent.EventParam & (FRONT_TAPE | CENTER_TAPE | RIGHT_TAPE) != 0) {
                                if ((TIMERS_GetTime() - initTimerTrigger) > 5000) {
                                    printf("\r\n LEFT ANGLE: %d", numRightAngleTurns);
                                    initTimerTrigger = TIMERS_GetTime();
                                    numRightAngleTurns += 1;
                                }
                            }
                        }
                    }
                    // COUNT 2 CORNER TURNS
                    if (numRightAngleTurns == 2) {
                        if (alignTape == RIGHT) {
                            if ((ThisEvent.EventParam & 0b010011) == 0b010011) {
                                SendDriveCommand1(STOP, 1);
                                nextState = IFZ;
                                makeTransition = TRUE;
                                ThisEvent.EventType = IFZ_DONE;
                                break;
                            } else {
                                SendDriveCommand1(SPIN_RIGHT, 1);
                            }
                        } else {
                            if ((ThisEvent.EventParam & 0b001011) == 0b001011) {
                                SendDriveCommand1(STOP, 1);
                                nextState = IFZ;
                                makeTransition = TRUE;
                                ThisEvent.EventType = IFZ_DONE;
                                break;
                            } else {
                                SendDriveCommand1(SPIN_LEFT, 1);
                            }
                        }
                    }
                    switch (ThisEvent.EventParam & 0b0011111) {
                        case (FRONT_TAPE): // STRAIGHT LINE FOLLOW
                        case (FRONT_TAPE | LEFT_TAPE):
                        case (FRONT_TAPE | RIGHT_TAPE):
                        case (FRONT_TAPE | CENTER_TAPE):
                        case (FRONT_TAPE | CENTER_TAPE | LEFT_TAPE):
                        case (FRONT_TAPE | CENTER_TAPE | RIGHT_TAPE):
                        case (FRONT_TAPE | CENTER_TAPE | BACK_TAPE):
                            SendDriveCommand1(FORWARD, 1);
                            correctionSequence = 0;
                            break;
                        case (FRONT_TAPE | CENTER_TAPE | LEFT_TAPE | BACK_TAPE):
                            SendDriveCommand1(FORWARD_SOFT_LEFT, 1);
                            correctionSequence = 0;
                            break;
                        case (FRONT_TAPE | CENTER_TAPE | RIGHT_TAPE | BACK_TAPE):
                            SendDriveCommand1(FORWARD_SOFT_RIGHT, 1);
                            correctionSequence = 0;
                            break;
                        case (CENTER_TAPE | BACK_TAPE): // TAPE CORRECTION
                            if (alignTape == RIGHT) {
                                if (correctionSequence != 2) {
                                    SendDriveCommand1(SPIN_LEFT, 1);
                                    correctionSequence = 1;
                                } else {
                                    SendDriveCommand1(SPIN_RIGHT, 1);
                                }
                            } else {
                                if (correctionSequence != 2) {
                                    SendDriveCommand1(SPIN_RIGHT, 1);
                                    correctionSequence = 1;
                                } else {
                                    SendDriveCommand1(SPIN_LEFT, 1);
                                }
                            }
                            break;
                        case (CENTER_TAPE | LEFT_TAPE | BACK_TAPE):
                            SendDriveCommand1(SPIN_LEFT, 1);
                            if (correctionSequence == 1) {
                                correctionSequence = 2;
                            }
                            break;
                        case (CENTER_TAPE | RIGHT_TAPE | BACK_TAPE):
                            SendDriveCommand1(SPIN_RIGHT, 1);
                            if (correctionSequence == 1) {
                                correctionSequence = 2;
                            }
                            break;
                        case (RIGHT_TAPE | CENTER_TAPE):
                        case (RIGHT_TAPE | BACK_TAPE):
                            SendDriveCommand1(SPIN_RIGHT, 1);
                            correctionSequence = 0;
                            break;
                        case (LEFT_TAPE | CENTER_TAPE):
                        case (LEFT_TAPE | BACK_TAPE):
                            SendDriveCommand1(SPIN_LEFT, 1);
                            correctionSequence = 0;
                            break;
                        case (BACK_TAPE):
                            SendDriveCommand1(BACKWARDS, 1);
                            break;
                        case (CENTER_TAPE):
                        case (CENTER_TAPE | LEFT_TAPE | RIGHT_TAPE): // TURNING
                        case (CENTER_TAPE | LEFT_TAPE | RIGHT_TAPE | BACK_TAPE):
                            if (alignTape == LEFT) {
                                SendDriveCommand1(SPIN_LEFT, 1);
                                break;
                            } else {
                                SendDriveCommand1(SPIN_RIGHT, 1);
                                break;
                            }
                            correctionSequence = 0;
                            break;
                        default:
                            break;
                    }
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == HSM_TIMER_2) {
                        startCounter = TRUE;
                    } else {
                        ThisEvent = RunNavigationSubSubHSM(ThisEvent);
                    }
                default:
                    break;
            }
            break;
        case IFZ: // IFZ IS REACHED
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    printf("\r\n === IFZ ===");
                    SendDriveCommand1(STOP, 1);
                    break;
                default:
                    break;
            }
            break;
    } // end switch on Current State
    if (makeTransition == TRUE) {
        RunIFZSubHSM(EXIT_EVENT);
        CurrentState = nextState;
        RunIFZSubHSM(ENTRY_EVENT);
    }
    ES_Tail();
    return ThisEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
ES_Event SendDriveCommand1(uint8_t command, uint8_t mode) {
    // keep track of number of (consecutive) left spins happening
    static uint8_t spinCounter = 0;
    if (command == SPIN_LEFT) {
        spinCounter += 1;
    } else if (command == SPIN_RIGHT) {
    } else {
        spinCounter == 0;
    }
    // if number of consecutive left spins are >= 3, drive forward instead
    if ((spinCounter >= 3) && (command == SPIN_LEFT)) {
        command = FORWARD;
        spinCounter = 0;
    }
    // pass command to subsubHSM
    ES_Event driveCommand;
    if (mode == 1) {
        driveCommand.EventType = DRIVE_EVENT_1;
    } else {
        driveCommand.EventType = DRIVE_EVENT_2;
    }
    driveCommand.EventParam = command;
    ES_Event returnEvent = RunNavigationSubSubHSM(driveCommand);
    return returnEvent;
}

void DrivePotorSpeeds(int dutyCycleA, int dutyCycleB) {
    PWM_SetDutyCycle(EN_A, dutyCycleA);
    PWM_SetDutyCycle(EN_B, dutyCycleB);

}