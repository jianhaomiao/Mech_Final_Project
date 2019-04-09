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
#include "AD.h"
#include "IO_Ports.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BeaconService.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define TIMER_TICKS     5   // 200Hz

#define NUM_EVENTS      20

#define THRESHOLD       835 // 825 was triggering at the wrong frequencies

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
uint8_t InitBeaconService(uint8_t Priority) {
    ES_Event ThisEvent;

    MyPriority = Priority;
    ES_Timer_InitTimer(BEACON_SERVICE_TIMER, TIMER_TICKS); // Set timer

    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

uint8_t PostBeaconService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event RunBeaconService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = BEACON_EVENT;

    static uint8_t eventHistory[NUM_EVENTS];
    static uint8_t prevEventParam;
    uint8_t currEventParam, sample, i;

    // service code starts here
    switch (ThisEvent.EventType) {
        case ES_INIT:
            break;
        case ES_NO_EVENT:
        case ES_TIMERACTIVE:
            break;
        case ES_TIMEOUT:
            // read beacon detector input pins
            if (AD_ReadADPin(BEACON_IN) >= THRESHOLD) {
                currEventParam = DETECTED;
                // printf("\r\n detected: %d", AD_ReadADPin(BEACON_IN));
            } else {
                currEventParam = NOT_DETECTED;
                // printf("\r\n not detected: %d", AD_ReadADPin(BEACON_IN));
            }
            // Check if event is valid
            uint8_t validEvent = TRUE;
            for (i = 0; i < NUM_EVENTS; i++) {
                if (currEventParam != eventHistory[i]) {
                    validEvent = FALSE;
                    break;
                }
            }
            // check if there is an event change
            if (currEventParam != prevEventParam) {
                ReturnEvent.EventType = BEACON_EVENT;
                ReturnEvent.EventParam = currEventParam;
                prevEventParam = currEventParam; // update history
#ifndef SERVICE_TEST // for test harness
                PostHSM(ReturnEvent);
#else
                PostBeaconService(ReturnEvent);
#endif   
            }
            // update history
            for (i = NUM_EVENTS - 1; i > 0; i--) {
                eventHistory[i] = eventHistory[i - 1];
            }
            eventHistory[0] = currEventParam;
            ES_Timer_InitTimer(BEACON_SERVICE_TIMER, TIMER_TICKS);
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

