/*
 * File: Navigation.h
 * Author: Jian Hao Miao
 * 
 * Project: Slugnificant Seven
 */

#ifndef NAVIGATION_H
#define NAVIGATION_H

/*******************************************************************************
 * PUBLIC FUNCTION                                                             *
 ******************************************************************************/
int8_t NavigationInit() {
    IO_PortsSetPortBits(PORTX, PIN3 | PIN4 | PIN5 | PIN6);
}

int8_t DriveForward();

int8_t DriveBackwards();

int8_t TurnLeft();

int8_t TurnRight();

int8_t EvadeObstacle();

#endif /* NAVIGATION_H */

