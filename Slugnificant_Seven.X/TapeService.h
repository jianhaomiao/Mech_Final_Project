/*
 * File: Service.h
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 * 
 * Project: Slugnificant Seven
 * Modified: Jian Hao Miao, Evan Plummer, Stephanie Lin
 */

#ifndef TapeService_H
#define TapeService_H

/*******************************************************************************
 * PUBLIC #INCLUDES                                                            *
 ******************************************************************************/
#include "ES_Configure.h"

/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/
// PORT DEFINES
//#define TAPE_PWR_OUT      PIN3           // PORTY
//#define TAPE_TOGGLE       PIN5

#define TAPE_PWR_OUT      PIN3 | PIN4           // PORTY
#define TAPE_TOGGLE       PIN5 | PIN6

#define TAPE_FRONT_IN     AD_PORTV3
#define TAPE_CENTER_IN    AD_PORTV4
#define TAPE_BACK_IN      AD_PORTV5
#define TAPE_LEFT_IN      AD_PORTV6
#define TAPE_RIGHT_IN     AD_PORTV7
#define TAPE_R_ANGLE_IN   AD_PORTV8
#define TAPE_L_ANGLE_IN   AD_PORTW3

// EVENT PARAM DEFINES
#define FRONT_TAPE         0b0000001 //  1
#define CENTER_TAPE        0b0000010 //  2
#define BACK_TAPE          0b0000100 //  4
#define LEFT_TAPE          0b0001000 //  8
#define RIGHT_TAPE         0b0010000 // 16
#define RIGHT_ANGLE_TAPE   0b0100000 // 32
#define LEFT_ANGLE_TAPE    0b1000000 // 64

/*******************************************************************************
 * PUBLIC TYPEDEFS                                                             *
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/
/**
 * @Function InitTapeService(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateService function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitTapeService(uint8_t Priority);

/**
 * @Function PostTapeService(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to. Remember to rename to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t PostTapeService(ES_Event ThisEvent);

/**
 * @Function RunTapeService(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the service,
 *        as this is called any time a new event is passed to the event queue. 
 * @note Remember to rename to something appropriate.
 *       Returns ES_NO_EVENT if the event have been "consumed." 
 * @author J. Edward Carryer, 2011.10.23 19:25 */
ES_Event RunTapeService(ES_Event ThisEvent);

#endif /* TapeService_H */