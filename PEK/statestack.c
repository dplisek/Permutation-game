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

void initStateStack()
{
    stateStack = (StateStack *)malloc(sizeof(StateStack));
    stateStack->allocated = 0;
    stateStack->size = 0;
}

void pushState(State *state)
{
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

State **popStatesOffBottom(int count)
{
    int i;
    State **states = (State **)malloc(sizeof(State *) * count);
    for (int i = 0; i < count; i++) {
        State *state = stateStack->states[i];
        states[i] = state;
    }
    for (i = 0; i < stateStack->size - count; i++) {
        stateStack->states[i] = stateStack->states[i + count];
    }
    stateStack->size -= count;
    return states;
}

State *stateAtIndex(int index)
{
    return stateStack->states[index];
}

void freeStateStack()
{
    free(stateStack->states);
    free(stateStack);
}
