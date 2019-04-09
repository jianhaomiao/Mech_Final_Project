/*
 * File: NavigationSubSubHSM.c
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 * 
 * Project: Slugnificant Seven
 * Modified: Jian Hao Miao, Evan Plummer, Stephanie Lin
 */

#ifndef Navigation_Sub_Sub_HSM_H
#define Navigation_Sub_Sub_HSM_H

/*******************************************************************************
 * PUBLIC #INCLUDES                                                            *
 ******************************************************************************/
#include "ES_Configure.h"
#include "pwm.h"
#include "IO_Ports.h"

/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/
#define EN_A           PWM_PORTY12
#define DIR_A                PIN11 // PORTY
#define EN_B           PWM_PORTY10
#define DIR_B                 PIN9 // PORTY

// Drive commands
#define FORWARD                     0
#define FORWARD_SOFT_LEFT           1
#define FORWARD_SOFT_RIGHT          2
#define PIVOT_FORWARD_LEFT          3
#define PIVOT_FORWARD_RIGHT         4

#define BACKWARDS                   5
#define BACKWARDS_SOFT_LEFT         6
#define BACKWARDS_SOFT_RIGHT        7
#define PIVOT_BACKWARDS_LEFT        8
#define PIVOT_BACKWARDS_RIGHT       9

#define SPIN_LEFT                  10
#define SPIN_RIGHT                 11

#define EVADE_LEFT                 12
#define EVADE_RIGHT                13

#define SWIVEL                     14
#define STOP                       15

#define FORWARD_TIMED_VERY_SHORT   16
#define BACKWARDS_TIMED_VERY_SHORT 17

#define FORWARD_TIMED_SHORT        18
#define BACKWARDS_TIMED_SHORT      19

#define FORWARD_TIMED_MEDIUM       20
#define BACKWARDS_TIMED_MEDIUM     21

#define FORWARD_TIMED_LONG         22
#define BACKWARDS_TIMED_LONG       23

#define HARD_TURN_45_LEFT          24
#define HARD_TURN_90_LEFT          25
#define HARD_TURN_45_RIGHT         26
#define HARD_TURN_56_RIGHT         27
#define HARD_TURN_90_RIGHT         28

#define FORWARD_EVADE_1            29
#define FORWARD_EVADE_2            30
#define FORWARD_EVADE_3            31
#define BACKWARDS_EVADE            32

/*******************************************************************************
 * PUBLIC TYPEDEFS                                                             *
 ******************************************************************************/


/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/
/**
 * @Function InitNavigationSubSubHSM(void)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateFSM function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitNavigationSubSubHSM(void);

/**
 * @Function RunNavigationSubSubHSM(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the heirarchical state
 *        machine, as this is called any time a new event is passed to the event
 *        queue. This function will be called recursively to implement the correct
 *        order for a state transition to be: exit current state -> enter next state
 *        using the ES_EXIT and ES_ENTRY events.
 * @note Remember to rename to something appropriate.
 *       The lower level state machines are run first, to see if the event is dealt
 *       with there rather than at the current level. ES_EXIT and ES_ENTRY events are
 *       not consumed as these need to pass pack to the higher level state machine.
 * @author J. Edward Carryer, 2011.10.23 19:25
 * @author Gabriel H Elkaim, 2011.10.23 19:25 */
ES_Event RunNavigationSubSubHSM(ES_Event ThisEvent);

#endif /* IFZ_Sub_HSM_H */

