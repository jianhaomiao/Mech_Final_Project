/*
 * File: HSM.c
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
#include "HSM.h"
#include "IFZSubHSM.h"
#include "TakeCoverSubHSM.h"

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
//Include any defines you need to do

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
typedef enum {
    InitPState,
    IFZ,
    TakeCover,
} HSMState_t;

static const char *StateNames[] = {
	"InitPState",
	"IFZ",
	"TakeCover",
};

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/


/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                            *
 ******************************************************************************/
static HSMState_t CurrentState = InitPState;
static uint8_t MyPriority;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/
uint8_t InitHSM(uint8_t Priority) {
    MyPriority = Priority;
    CurrentState = InitPState;

    // post initial transition
    if (ES_PostToService(MyPriority, INIT_EVENT) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

uint8_t PostHSM(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event RunHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE;
    HSMState_t nextState;

    static uint8_t start;

    // ES_Tattle();

    switch (CurrentState) {
        case InitPState:
            if (ThisEvent.EventType == ES_INIT) {
                // Initiate sub HSMs
                InitIFZSubHSM();
                InitTakeCoverSubHSM();

                // Initiate subsub HSMs
                InitNavigationSubSubHSM();

                nextState = IFZ; // FIRST STATE
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
        case IFZ: // Navigate to IFZ
            switch (ThisEvent.EventType) {
                case BUMPER_EVENT:
                    if (start == 0) {
                        start = 1;
                        ThisEvent.EventType = START_BOT;
                        ThisEvent = RunIFZSubHSM(ThisEvent);
                        break;
                    }
                case TAPE_EVENT:
                case BEACON_EVENT:
                case ES_TIMEOUT:
                    ThisEvent = RunIFZSubHSM(ThisEvent);
                    switch (ThisEvent.EventType) {
                        case IFZ_DONE:
                            printf("\r\n = transition =");
                            nextState = TakeCover;
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
        case TakeCover: // Advance, Take cover, and Fire
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ThisEvent.EventType = START_BOT;
                    ThisEvent = RunTakeCoverSubHSM(ThisEvent);
                    break;
                case TAPE_EVENT:
                case BUMPER_EVENT:
                case BEACON_EVENT:
                case ES_TIMEOUT:
                    ThisEvent = RunTakeCoverSubHSM(ThisEvent);
                    switch (ThisEvent.EventType) {
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    if (makeTransition == TRUE) {
        RunHSM(EXIT_EVENT);
        CurrentState = nextState;
        RunHSM(ENTRY_EVENT);
    }

    ES_Tail();
    return ThisEvent;
}


