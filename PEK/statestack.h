//
//  statestack.h
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#ifndef __PEK__statestack__
#define __PEK__statestack__

#include "statestack.h"

typedef struct state {
    struct state *parent;
    int blankIndex;
    int depth;
} State;

typedef struct {
    State **states;
    int size;
    int allocated;
} StateStack;

void pushState(State *state);
State *popState();
State **popStatesOffBottom(int count);
State *stateAtIndex(int index);
void freeStateStack();

#endif /* defined(__PEK__statestack__) */
