//
//  statestack.c
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#include "statestack.h"
#include <stdlib.h>
#include "config.h"

extern StateStack *stateStack;

void pushState(State *state)
{
    if (!stateStack) {
        stateStack = (StateStack *)malloc(sizeof(StateStack));
        stateStack->allocated = 0;
        stateStack->size = 0;
    }
    if (!stateStack->allocated) {
        stateStack->states = (State **)malloc(sizeof(State *) * STACK_INITIAL_ALLOC);
        stateStack->allocated = STACK_INITIAL_ALLOC;
    } else if (stateStack->size + 1 > stateStack->allocated) {
        int newAlloc = stateStack->allocated * stateStack->allocated;
        stateStack->states = (State **)realloc(stateStack->states, sizeof(State *) * newAlloc);
        stateStack->allocated = newAlloc;
    }
    stateStack->states[stateStack->size] = state;
    stateStack->size++;
}

State *popState()
{
    State *state = stateStack->states[stateStack->size - 1];
    stateStack->size--;
    return state;
}

State *popStateOffBottom()
{
    int i;
    State *state = stateStack->states[0];
    for (i = 0; i < stateStack->size - 1; i++) {
        stateStack->states[i] = stateStack->states[i + 1];
    }
    stateStack->size--;
    return state;
}

void freeStateStack()
{
    free(stateStack->states);
    free(stateStack);
}
