/*
 * File: AdvanceSubHSM.c
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 * 
 * Project: Slugnificant Seven
 * Modified: Jian Hao Miao, Evan Plummer, Stephanie Lin
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "HSM.h"
#include "AdvanceSubHSM.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
typedef enum {
    InitPSubState,
} AdvanceSubHSMState_t;

static const char *StateNames[] = {
	"InitPSubState",
};

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/


/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
static AdvanceSubHSMState_t CurrentState = InitPSubState;
static uint8_t MyPriority;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/
uint8_t InitAdvanceSubHSM(void) {
    ES_Event returnEvent;

    CurrentState = InitPSubState;
    returnEvent = RunAdvanceSubHSM(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

ES_Event RunAdvanceSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE;
    AdvanceSubHSMState_t nextState;

    ES_Tattle();

    switch (CurrentState) {
        case InitPSubState:
            if (ThisEvent.EventType == ES_INIT) {
                // DO SOMETHING
            }
            break;
        default:
            break;
    } // end switch on Current State

    if (makeTransition == TRUE) {
        RunAdvanceSubHSM(EXIT_EVENT);
        CurrentState = nextState;
        RunAdvanceSubHSM(ENTRY_EVENT);
    }

    ES_Tail();
    return ThisEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/
