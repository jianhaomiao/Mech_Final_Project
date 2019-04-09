/*
 * File: NavigationSubSubHSM.c
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
#include "IO_Ports.h"
#include "HSM.h"
#include "NavigationSubSubHSM.h"
#include "pwm.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define TIMER_FORWARD_EVADE_1    2450 // forward timer for EVADE
#define TIMER_FORWARD_EVADE_2    2100
#define TIMER_FORWARD_EVADE_3    1500

#define TIMER_BACKWARDS_EVADE     530 // backwards timer for EVADE

#define TIMER_DRIVE_TICKS_SHORT   250 // timed drive
#define TIMER_DRIVE_TICKS_MEDIUM 1000
#define TIMER_DRIVE_TICKS_LONG   3500

#define TIMER_TURN_TICKS_45      390 // 45 degree turn -50
#define TIMER_TURN_TICKS_56      1 // 67 degree turn
#define TIMER_TURN_TICKS_90      780 // 90 degree turn -100

#define FORWARD_DRIVE              0 // directions
#define BACKWARDS_DRIVE            1
#define LEFT_TURN                  0
#define RIGHT_TURN                 1

#define DRIVE_SPEED_1            700 // motor drive speeds (default)
#define DRIVE_SPEED_2            500
#define DRIVE_SPEED_3            300

#define SWIVEL_TIME_1            260 // swivel delays

typedef enum {
    InitPSubSubState,
    Navigate,
    DriveStraight,
    DriveSoftTurn,
    DriveSpin,
    DrivePivot,
    DriveHardTurn,
    DriveEvade,
    DriveSwivel,
    DriveBackwards,
    BackwardsTurn,
    DriveStop,
} SubSubHSMState_t;

static const char *StateNames[] = {
	"InitPSubSubState",
	"Navigate",
	"DriveStraight",
	"DriveSoftTurn",
	"DriveSpin",
	"DrivePivot",
	"DriveHardTurn",
	"DriveEvade",
	"DriveSwivel",
	"DriveBackwards",
	"BackwardsTurn",
	"DriveStop",
};

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
SubSubHSMState_t DetermineNextState(SubSubHSMState_t);
void DriveMotorSpeeds(int dutyCycleA, int dutyCycleB);

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
static SubSubHSMState_t CurrentState = Navigate;
static uint8_t MyPriority;

static uint8_t RampDownSpeed;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/
uint8_t InitNavigationSubSubHSM(void) {
    ES_Event returnEvent;

    // post initial transition
    CurrentState = InitPSubSubState;
    returnEvent = RunNavigationSubSubHSM(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

ES_Event RunNavigationSubSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE;
    SubSubHSMState_t nextState;

    static uint8_t direction;
    static uint8_t turn;
    static uint8_t swivelDirection;
    static uint16_t driveTime;

    ES_Tattle();

    switch (CurrentState) {
        case InitPSubSubState:
            if (ThisEvent.EventType == ES_INIT) {
                nextState = Navigate;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        case Navigate:
            if (ThisEvent.EventType == DRIVE_EVENT_1) {
                RampDownSpeed = 0;
            } else if (ThisEvent.EventType == DRIVE_EVENT_2) {
                RampDownSpeed = 1; // SLOWS DOWN DEFAULT SPEED BY 200
            }
            switch (ThisEvent.EventType) {
                case DRIVE_EVENT_1:
                case DRIVE_EVENT_2:
                    // DETERMINE FORWARD OR BACKWARDS
                    switch (ThisEvent.EventParam) {
                        case FORWARD:
                        case FORWARD_TIMED_SHORT: // TO CHANGE
                        case FORWARD_TIMED_MEDIUM:
                        case FORWARD_TIMED_LONG: // TO CHANGE
                        case FORWARD_SOFT_LEFT:
                        case FORWARD_SOFT_RIGHT:
                        case PIVOT_FORWARD_LEFT:
                        case PIVOT_FORWARD_RIGHT:
                        case FORWARD_EVADE_1:
                        case FORWARD_EVADE_2:
                        case FORWARD_EVADE_3:
                            direction = FORWARD_DRIVE;
                            break;
                        case BACKWARDS:
                        case BACKWARDS_TIMED_SHORT: // TO CHANGE
                        case BACKWARDS_TIMED_MEDIUM:
                        case BACKWARDS_TIMED_LONG: // TO CHANGE
                        case BACKWARDS_SOFT_LEFT:
                        case BACKWARDS_SOFT_RIGHT:
                        case PIVOT_BACKWARDS_LEFT:
                        case PIVOT_BACKWARDS_RIGHT:
                        case BACKWARDS_EVADE:
                            direction = BACKWARDS_DRIVE;
                            break;
                        default:
                            direction = 0;
                    }
                    // DETERMINE LEFT OR RIGHT
                    switch (ThisEvent.EventParam) {
                        case FORWARD_SOFT_LEFT:
                        case BACKWARDS_SOFT_LEFT:
                        case PIVOT_FORWARD_LEFT:
                        case PIVOT_BACKWARDS_LEFT:
                        case HARD_TURN_45_LEFT:
                        case HARD_TURN_90_LEFT:
                        case SPIN_LEFT:
                        case EVADE_LEFT:
                            turn = LEFT_TURN;
                            break;
                        case FORWARD_SOFT_RIGHT:
                        case BACKWARDS_SOFT_RIGHT:
                        case PIVOT_FORWARD_RIGHT:
                        case PIVOT_BACKWARDS_RIGHT:
                        case HARD_TURN_45_RIGHT:
                        case HARD_TURN_56_RIGHT:
                        case HARD_TURN_90_RIGHT:
                        case SPIN_RIGHT:
                        case EVADE_RIGHT:
                            turn = RIGHT_TURN;
                            break;
                        default:
                            turn = 0;
                    }
                    // DETERMINE TIMERS
                    switch (ThisEvent.EventParam) {
                        case FORWARD_TIMED_SHORT:
                        case BACKWARDS_TIMED_SHORT:
                            driveTime = TIMER_DRIVE_TICKS_SHORT;
                            break;
                        case FORWARD_TIMED_MEDIUM:
                        case BACKWARDS_TIMED_MEDIUM:
                            driveTime = TIMER_DRIVE_TICKS_MEDIUM;
                            break;
                        case FORWARD_TIMED_LONG:
                        case BACKWARDS_TIMED_LONG:
                            driveTime = TIMER_DRIVE_TICKS_LONG;
                            break;
                        case HARD_TURN_45_LEFT:
                        case HARD_TURN_45_RIGHT:
                            driveTime = TIMER_TURN_TICKS_45;
                            break;
                        case HARD_TURN_56_RIGHT:
                            driveTime = TIMER_TURN_TICKS_56;
                            break;
                        case HARD_TURN_90_LEFT:
                        case HARD_TURN_90_RIGHT:
                            driveTime = TIMER_TURN_TICKS_90;
                            break;
                        case FORWARD_EVADE_1:
                            driveTime = TIMER_FORWARD_EVADE_1;
                            break;
                        case FORWARD_EVADE_2:
                            driveTime = TIMER_FORWARD_EVADE_2;
                            break;
                        case FORWARD_EVADE_3:
                            driveTime = TIMER_FORWARD_EVADE_3;
                            break;
                        case BACKWARDS_EVADE:
                            driveTime = TIMER_BACKWARDS_EVADE;
                            break;
                        default:
                            driveTime = 0;
                            break;
                    }
                    // DETERMINE NEXT STATE
                    switch (ThisEvent.EventParam) {
                        case FORWARD:
                        case FORWARD_TIMED_SHORT:
                        case FORWARD_TIMED_MEDIUM:
                        case FORWARD_TIMED_LONG:
                        case BACKWARDS:
                        case BACKWARDS_TIMED_SHORT:
                        case BACKWARDS_TIMED_MEDIUM:
                        case BACKWARDS_TIMED_LONG:
                        case FORWARD_EVADE_1:
                        case FORWARD_EVADE_2:
                        case FORWARD_EVADE_3:
                        case BACKWARDS_EVADE:
                            nextState = DriveStraight;
                            break;
                        case FORWARD_SOFT_LEFT:
                        case FORWARD_SOFT_RIGHT:
                        case BACKWARDS_SOFT_LEFT:
                        case BACKWARDS_SOFT_RIGHT:
                            nextState = DriveSoftTurn;
                            break;
                        case PIVOT_FORWARD_LEFT:
                        case PIVOT_FORWARD_RIGHT:
                        case PIVOT_BACKWARDS_LEFT:
                        case PIVOT_BACKWARDS_RIGHT:
                            nextState = DrivePivot;
                            break;
                        case SPIN_LEFT:
                        case SPIN_RIGHT:
                            nextState = DriveSpin;
                            break;
                        case HARD_TURN_45_RIGHT:
                        case HARD_TURN_56_RIGHT:
                        case HARD_TURN_90_RIGHT:
                        case HARD_TURN_45_LEFT:
                        case HARD_TURN_90_LEFT:
                            nextState = DriveHardTurn;
                            break;
                        case EVADE_LEFT:
                        case EVADE_RIGHT:
                            nextState = DriveEvade;
                            break;
                        case SWIVEL:
                            nextState = DriveSwivel;
                            break;
                        case STOP:
                            nextState = DriveStop;
                            break;
                        default:
                            break;
                    }
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    break;
            }
            break;
        case DriveStraight:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    if (driveTime != 0) {
                        // printf(" - %d", driveTime);
                        ES_Timer_InitTimer(HSM_TIMER_1, driveTime);
                        if (direction == FORWARD_DRIVE) {
                            // printf("\r\nI AM FORWARD");
                            DriveMotorSpeeds(1000, 1000);
                            IO_PortsSetPortBits(PORTY, DIR_A | DIR_B);
                        } else {
                            // printf("\r\nI AM BACKWARDS");
                            DriveMotorSpeeds(800, 800);
                            IO_PortsClearPortBits(PORTY, DIR_A | DIR_B);
                        }
                    } else {
                        if (direction == FORWARD_DRIVE) {
                            // printf("\r\nI AM FORWARD");
                            DriveMotorSpeeds(DRIVE_SPEED_1, DRIVE_SPEED_1);
                            IO_PortsSetPortBits(PORTY, DIR_A | DIR_B);
                        } else {
                            // printf("\r\nI AM BACKWARDS");
                            DriveMotorSpeeds(DRIVE_SPEED_2, DRIVE_SPEED_2);
                            IO_PortsClearPortBits(PORTY, DIR_A | DIR_B);
                        }
                        nextState = Navigate;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_TIMEOUT:
                    if (driveTime != 0) {
                        nextState = DriveStop;
                    } else {
                        nextState = Navigate;
                    }
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                default:
                    break;
            }
            break;
        case DriveSoftTurn:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    if (turn == RIGHT_TURN) {
                        // printf("\r\nI AM SOFT RIGHT");
                        DriveMotorSpeeds(DRIVE_SPEED_1, DRIVE_SPEED_2);
                    } else {
                        // printf("\r\nI AM SOFT LEFT");
                        DriveMotorSpeeds(DRIVE_SPEED_2, DRIVE_SPEED_1);
                    }
                    if (direction == FORWARD_DRIVE) {
                        // printf(" - FORWARD");
                        IO_PortsSetPortBits(PORTY, DIR_A | DIR_B);
                    } else {
                        // printf(" - BACKWARDS");
                        IO_PortsClearPortBits(PORTY, DIR_A | DIR_B);
                    }
                    nextState = Navigate;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                default:
                    break;
            }
            break;
        case DrivePivot:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    if (turn == RIGHT_TURN) {
                        // printf("\r\nI AM PIVOT RIGHT");
                        DriveMotorSpeeds(DRIVE_SPEED_1, DRIVE_SPEED_3); // PIVOT MODIFIED

                        IO_PortsSetPortBits(PORTY, DIR_A);
                        IO_PortsClearPortBits(PORTY, DIR_B);
                    } else {
                        // printf("\r\nI AM PIVOT LEFT");
                        DriveMotorSpeeds(DRIVE_SPEED_3, DRIVE_SPEED_1); // PIVOT MODIFIED

                        IO_PortsClearPortBits(PORTY, DIR_A);
                        IO_PortsSetPortBits(PORTY, DIR_B);
                    }
                    // if (direction == FORWARD_DRIVE) {
                    // printf(" - FORWARD");
                    // IO_PortsSetPortBits(PORTY, DIR_A);
                    // IO_PortsSetPortBits(PORTY, DIR_B);
                    // } else {
                    // printf(" - BACKWARDS");
                    // IO_PortsClearPortBits(PORTY, DIR_A);
                    // IO_PortsClearPortBits(PORTY, DIR_B);
                    // }
                    nextState = Navigate;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    break;
            }
            break;
        case DriveHardTurn:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    if (turn == RIGHT_TURN) {
                        // printf("\r\nI AM HARD RIGHT");
                        DriveMotorSpeeds(1000, 1000);

                        IO_PortsSetPortBits(PORTY, DIR_A);
                        IO_PortsClearPortBits(PORTY, DIR_B);
                    } else {
                        // printf("\r\nI AM HARD LEFT");
                        DriveMotorSpeeds(1000, 1000);

                        IO_PortsClearPortBits(PORTY, DIR_A);
                        IO_PortsSetPortBits(PORTY, DIR_B);
                    }
                    if (driveTime == TIMER_TURN_TICKS_90) {
                        // printf(" - 90");
                    } else if (driveTime == TIMER_TURN_TICKS_56) {
                        // printf(" - 56");
                    } else {
                        // printf(" - 45");
                    }
                    ES_Timer_InitTimer(HSM_TIMER_1, driveTime);
                    break;
                case ES_TIMEOUT:
                    // printf("\r\nHARD TURN TIMEOUT");
                    nextState = DriveStop;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                default:
                    break;
            }
            break;
        case DriveSpin:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    if (turn == RIGHT_TURN) {
                        // printf("\r\nI AM SPIN RIGHT");
                        DriveMotorSpeeds(DRIVE_SPEED_2, DRIVE_SPEED_2);

                        IO_PortsSetPortBits(PORTY, DIR_A);
                        IO_PortsClearPortBits(PORTY, DIR_B);
                    } else {
                        // printf("\r\nI AM SPIN LEFT");
                        DriveMotorSpeeds(DRIVE_SPEED_2, DRIVE_SPEED_2);

                        IO_PortsSetPortBits(PORTY, DIR_B);
                        IO_PortsClearPortBits(PORTY, DIR_A);
                    }
                    nextState = Navigate;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    break;
            }
            break;
        case DriveSwivel:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    if (swivelDirection == 0) {
                        // printf("\r\n SWIVEL LEFT");
                        DriveMotorSpeeds(DRIVE_SPEED_2, DRIVE_SPEED_2);

                        IO_PortsSetPortBits(PORTY, DIR_B); // SPIN LEFT
                        IO_PortsClearPortBits(PORTY, DIR_A);

                        ES_Timer_InitTimer(HSM_TIMER_1, SWIVEL_TIME_1);
                    } else {
                        // printf("\r\n SWIVEL RIGHT");
                        DriveMotorSpeeds(DRIVE_SPEED_2, DRIVE_SPEED_2);

                        IO_PortsSetPortBits(PORTY, DIR_A); // SPIN RIGHT
                        IO_PortsClearPortBits(PORTY, DIR_B);

                        ES_Timer_InitTimer(HSM_TIMER_1, SWIVEL_TIME_1);
                    }
                    break;
                case SWIVEL_STOP:
                    // printf("\r\n SWIVEL STOP");
                    nextState = DriveStop;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if (swivelDirection == 0) {
                        swivelDirection = 1;
                    } else {
                        swivelDirection = 0;
                    }
                    nextState = DriveSwivel;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                default:
                    break;
            }
            break;
        case DriveStop:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    // printf("\r\nI AM STOP");
                    PWM_SetDutyCycle(EN_A, 0);
                    PWM_SetDutyCycle(EN_B, 0);

                    nextState = Navigate;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                default:
                    break;
            }
            break;
        default:
            break;
    } // end switch on Current State

    if (makeTransition == TRUE) {
        RunNavigationSubSubHSM(EXIT_EVENT);
        CurrentState = nextState;
        RunNavigationSubSubHSM(ENTRY_EVENT);
    }

    ES_Tail();
    return ThisEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
void DriveMotorSpeeds(int dutyCycleA, int dutyCycleB) {
    if (RampDownSpeed == 0) {
        PWM_SetDutyCycle(EN_A, dutyCycleA);
        PWM_SetDutyCycle(EN_B, dutyCycleB);
    } else {
        PWM_SetDutyCycle(EN_A, dutyCycleA - 200);
        PWM_SetDutyCycle(EN_B, dutyCycleB - 200);
    }
}