//
//  action.c
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#include "action.h"
#include "statestack.h"
#include <stdio.h>
#include <stdlib.h>
#include "output.h"
#include "followup.h"

extern int *gameBoard;
extern int maxDepth;
extern int minDepth;
extern State *previousState;

void swapIndices(int i1, int i2)
{
    int temp = gameBoard[i1];
    gameBoard[i1] = gameBoard[i2];
    gameBoard[i2] = temp;
}

BOOL backUpAndFindCommonParent(State *state, State *previousState)
{
    State *stateToFree;
    while (previousState != state->parent) {
        if (!previousState->parent) {
            printf("Reached top of state space while searching for common parent. That is an error.\n");
            return NO;
        }
        swapIndices(previousState->blankIndex, previousState->parent->blankIndex);
        stateToFree = previousState;
        previousState = previousState->parent;
        free(stateToFree);
    }
    return YES;
}

BOOL makesSenseToGoDeeper(State *state)
{
    return state->depth < maxDepth && state->depth < minDepth - 1;
}

BOOL evaluateNextStackState()
{
    State *state = popState();
    if (state->parent) {
        if (!backUpAndFindCommonParent(state, previousState)) return NO;
        swapIndices(state->parent->blankIndex, state->blankIndex);
    }
    if (isFinal(state)) {
        saveResultIfBetter(state);
    } else if (makesSenseToGoDeeper(state)) {
        pushFollowupStates(state);
    }
    previousState = state;
    return YES;
}
