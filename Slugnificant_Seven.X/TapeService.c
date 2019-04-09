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
#include <stdlib.h>
#include "BOARD.h"
#include "AD.h"
#include "pwm.h"
#include "LED.h"
#include "IO_Ports.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "TapeService.h"

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define TIMER_TICKS      3 // 200 Hz

#define NUM_SENSORS      8
#define NUM_EVENTS       3 // moving average

#define THRESHOLD_1       23 // FOLLOWING TAPES
#define THRESHOLD_2       10 // CORNER ANGLES

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/
static uint8_t MyPriority;

static uint8_t flag; // for collecting data on high and low

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/
uint8_t InitTapeService(uint8_t Priority) {
    ES_Event ThisEvent;

    MyPriority = Priority;

    ES_Timer_InitTimer(TAPE_SERVICE_TIMER, TIMER_TICKS); // Set timer
    IO_PortsSetPortBits(PORTY, TAPE_PWR_OUT); // power tape sensors

    PWM_SetFrequency(MIN_PWM_FREQ); // set PWM 500 Hz at 50% duty cycle

    flag = 0; // initialize flag

    // post initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

uint8_t PostTapeService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event RunTapeService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assumes no errors

    static uint16_t eventHistory[NUM_EVENTS];

    static uint16_t sampleHigh[NUM_SENSORS], sampleLow[NUM_SENSORS];
    static uint16_t prevEventParam;

    uint16_t currEventParam, sample, i, j;

    // service code starts here
    switch (ThisEvent.EventType) {
        case ES_INIT:
            break;
        case ES_NO_EVENT:
        case ES_TIMERACTIVE:
            break;
        case ES_TIMEOUT:
            // read tape sensor input pins
            if (flag == 0) {
                sampleHigh[0] = AD_ReadADPin(TAPE_FRONT_IN);
                sampleHigh[1] = AD_ReadADPin(TAPE_CENTER_IN);
                sampleHigh[2] = AD_ReadADPin(TAPE_BACK_IN);
                sampleHigh[3] = AD_ReadADPin(TAPE_LEFT_IN);
                sampleHigh[4] = AD_ReadADPin(TAPE_RIGHT_IN);
                sampleHigh[5] = AD_ReadADPin(TAPE_R_ANGLE_IN);
                sampleHigh[6] = AD_ReadADPin(TAPE_L_ANGLE_IN);

                flag = 1;

                IO_PortsSetPortBits(PORTY, TAPE_TOGGLE);
            } else {
                sampleLow[0] = AD_ReadADPin(TAPE_FRONT_IN);
                sampleLow[1] = AD_ReadADPin(TAPE_CENTER_IN);
                sampleLow[2] = AD_ReadADPin(TAPE_BACK_IN);
                sampleLow[3] = AD_ReadADPin(TAPE_LEFT_IN);
                sampleLow[4] = AD_ReadADPin(TAPE_RIGHT_IN);
                sampleLow[5] = AD_ReadADPin(TAPE_R_ANGLE_IN) + 5;
                sampleLow[6] = AD_ReadADPin(TAPE_L_ANGLE_IN) + 5;

                IO_PortsClearPortBits(PORTY, TAPE_TOGGLE);

                // maintain running average of NUM_SAMPLES samples
                currEventParam = 0;
                for (i = 0; i < NUM_SENSORS; i++) {
                    sample = abs(sampleHigh[i] - sampleLow[i]); // sync sample
                    if (sample <= THRESHOLD_1) {
                        if (i == 0) {
                            currEventParam |= FRONT_TAPE;
                            // printf("\r\n FRONT: %d", sample);
                        } else if (i == 1) {
                            currEventParam |= CENTER_TAPE;
                            // printf("\r\nCENTER: %d", sample);
                        } else if (i == 2) {
                            currEventParam |= BACK_TAPE;
                            // printf("\r\nBACK: %d", sample);
                        } else if (i == 3) {
                            currEventParam |= LEFT_TAPE;
                            // printf("\r\nLEFT: %d", sample);
                        } else if (i == 4) {
                            currEventParam |= RIGHT_TAPE;
                            // printf("\r\nRIGHT: %d", sample);
                        }
                    }
                    if (sample <= THRESHOLD_2) {
                        if (i == 5) {
                            currEventParam |= RIGHT_ANGLE_TAPE;
                            // printf("\r\nRIGHT ANGLE: %d", sample);
                        } else if (i == 6) {
                            currEventParam |= LEFT_ANGLE_TAPE;
                            // printf("\r\nLEFT ANGLE: %d", sample);
                        }
                    }
                }
                // Check if event is valid
                uint8_t validEvent = TRUE;
                for (j = 0; j < NUM_EVENTS; j++) {
                    if (currEventParam != eventHistory[j]) {
                        validEvent = FALSE;
                        break;
                    }
                }
                // check if there is an event change
                if ((currEventParam != prevEventParam) & validEvent) {
                    ReturnEvent.EventType = TAPE_EVENT;
                    ReturnEvent.EventParam = currEventParam;
                    prevEventParam = currEventParam;

                    // DEBUG WITH LEDs
                    uint8_t bank2Val = 0;
                    if ((currEventParam & FRONT_TAPE) == FRONT_TAPE) {
                        LED_SetBank(LED_BANK3, 0b1111);
                    } else {
                        LED_SetBank(LED_BANK3, 0);
                    }
                    if ((currEventParam & BACK_TAPE) == BACK_TAPE) {
                        LED_SetBank(LED_BANK1, 0b1111);
                    } else {
                        LED_SetBank(LED_BANK1, 0);
                    }
                    if ((currEventParam & CENTER_TAPE) == CENTER_TAPE) {
                        bank2Val |= 0b0110;
                    }
                    if ((currEventParam & LEFT_TAPE) == LEFT_TAPE) {
                        bank2Val |= 0b1000;
                    }
                    if ((currEventParam & RIGHT_TAPE) == RIGHT_TAPE) {
                        bank2Val |= 0b0001;
                    }
                    LED_SetBank(LED_BANK2, bank2Val);
                    LED_AddBanks(LED_BANK1 | LED_BANK2 | LED_BANK3);

#ifndef SERVICE_TEST // for test harness
                    PostHSM(ReturnEvent);
#else
                    PostTapeService(ReturnEvent);
#endif
                }
                // update history
                for (i = NUM_EVENTS - 1; i > 0; i--) {
                    eventHistory[i] = eventHistory[i - 1];
                }
                eventHistory[0] = currEventParam;
                flag = 0;
            }
            ES_Timer_InitTimer(TAPE_SERVICE_TIMER, TIMER_TICKS);
            break;

#ifdef SERVICE_TEST // for test harness 
        default:
            //            printf("\r\nEvent: %s\tParam: 0x%X",
            //                    EventNames[ThisEvent.EventType], ThisEvent.EventParam);
            break;
#endif
    }
    return ReturnEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONs                                                           *
 ******************************************************************************/

