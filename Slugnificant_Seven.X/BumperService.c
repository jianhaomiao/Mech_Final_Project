/*
 * File: Service.c
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
#include "BOARD.h"
#include "IO_Ports.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BumperService.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define TIMER_TICKS     5 // 200Hz

#define NUM_EVENTS      20

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/
static uint8_t MyPriority;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/
uint8_t InitBumperService(uint8_t Priority) {
    ES_Event ThisEvent;

    MyPriority = Priority;

    ES_Timer_InitTimer(BUMPER_SERVICE_TIMER, TIMER_TICKS); // Set timer
    IO_PortsSetPortBits(PORTZ, BUMPER_PWR_OUT); // power tape sensors

    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

uint8_t PostBumperService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event RunBumperService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    static uint8_t eventHistory[NUM_EVENTS];
    static uint8_t prevEventParam;
    uint8_t currEventParam;
    int i;

    // service code starts here
    switch (ThisEvent.EventType) {
        case ES_INIT:
            break;
        case ES_NO_EVENT:
        case ES_TIMERACTIVE:
            break;
        case ES_TIMEOUT:
            // read bumper input pins
            currEventParam = (IO_PortsReadPort(PORTZ) & 0b0011000) >> 3;
            
            // Check if event is valid
            uint8_t validEvent = TRUE;
            for (i = 0; i < NUM_EVENTS; i++) {
                if (currEventParam != eventHistory[i]) {
                    validEvent = FALSE;
                    break;
                }
            }
            // check if there is an event change
            if ((currEventParam != prevEventParam) & validEvent) {
                ReturnEvent.EventType = BUMPER_EVENT;
                ReturnEvent.EventParam = currEventParam;
                prevEventParam = currEventParam;

#ifndef SERVICE_TEST // for test harness
                PostHSM(ReturnEvent);
#else
                PostBumperService(ReturnEvent);
#endif
            }
            // update history
            for (i = NUM_EVENTS - 1; i > 0; i--) {
                eventHistory[i] = eventHistory[i - 1];
            }
            eventHistory[0] = currEventParam;
            ES_Timer_InitTimer(BUMPER_SERVICE_TIMER, TIMER_TICKS);
            break;

#ifdef SERVICE_TEST // for test harness 
        default:
            printf("\r\nEvent: %s\tParam: 0x%X",
                    EventNames[ThisEvent.EventType], ThisEvent.EventParam);
            break;
#endif
    }

    return ReturnEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONs                                                           *
 ******************************************************************************/

