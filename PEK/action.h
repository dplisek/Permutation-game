//
//  action.h
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#ifndef __PEK__action__
#define __PEK__action__

#include "bool.h"
#include "statestack.h"

void swapIndices(int i1, int i2);
void forwardToState(State *state);
void evaluateNextStackState();
void copyGameBoard(int *src, int *target);

#endif /* defined(__PEK__action__) */
