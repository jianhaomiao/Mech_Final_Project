/*
 * File: Navigation.h
 * Author: Jian Hao Miao
 * 
 * Project: Slugnificant Seven
 */

#include "BOARD.h"
#include "IO_Ports.h"

/*******************************************************************************
 * PRIVATE #DEFINES                                                            *
 ******************************************************************************/
#define EN_A    PIN3
#define DIR_A   PIN4
#define EN_B    PIN5
#define DIR_B   PIN6

/*******************************************************************************
 * PUBLIC FUNCTION                                                             *
 ******************************************************************************/
char NavigationInit() {
    IO_PortsSetPortBits(PORTX, EN_A | DIR_A | EN_B | DIR_B);
    return SUCCESS;
}

char DriveForward() {
    IO_PortsSetPortBits(PORTX, EN_A | DIR_A | EN_B | DIR_B);
    return SUCCESS;
}

char DriveBackwards() {
    IO_PortsSetPortBits(PORTX, EN_A | DIR_A | EN_B | DIR_B);
    IO_PortsTogglePortBits(PORTX, DIR_A | DIR_B);
    return SUCCESS;
}

char TurnLeft() {

}

char TurnRight();

char EvadeObstacle();