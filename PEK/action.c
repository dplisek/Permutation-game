//
//  action.c
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#include "action.h"
#include <stdio.h>
#include <stdlib.h>
#include "output.h"
#include "followup.h"
#include "logging.h"

extern int *gameBoard;
extern int maxDepth;
extern int minDepth;
extern State *previousState;
extern int gameBoardFieldCount;

#ifdef DEBUG
extern int allocatedStates;
#endif

void swapIndices(int i1, int i2)
{
    int temp = gameBoard[i1];
    gameBoard[i1] = gameBoard[i2];
    gameBoard[i2] = temp;
    LOG("Swapped indices %d and %d.\n", i1, i2);
}

void backUpAndFindCommonParent(State *state, State *previousState)
{
    State *stateToFree;
    while (previousState != state->parent) {
        if (!previousState->parent) {
            fprintf(stderr, "Reached top of state space while searching for common parent. That is an error.\n");
            exit(EXIT_FAILURE);
        }
        swapIndices(previousState->blankIndex, previousState->parent->blankIndex);
        stateToFree = previousState;
        previousState = previousState->parent;
        free(stateToFree);
#ifdef DEBUG
        allocatedStates--;
#endif
    }
}

void forwardToState(State *state)
{
    int i, arraySize = state->depth + 1;
    State **states = malloc(sizeof(State *) * arraySize);
    for (i = arraySize - 1; i >= 0; i--) {
        states[i] = state;
        state = state->parent;
    }
    for (i = 0; i < arraySize - 1; i++) {
        swapIndices(states[i]->blankIndex, states[i + 1]->blankIndex);
    }
    free(states);
}

BOOL makesSenseToGoDeeper(State *state)
{
    return state->depth < maxDepth && state->depth < minDepth - 1;
}

void evaluateNextStackState()
{
    State *state = popState();
    LOG("Evaluating state of depth %d with blankIndex %d, whose parent (if exists) has blankIndex %d.\n", state->depth, state->blankIndex, state->parent ? state->parent->blankIndex : -1);
    if (state->parent) {
        backUpAndFindCommonParent(state, previousState);
        swapIndices(state->parent->blankIndex, state->blankIndex);
    }
#ifdef DEBUG
    if (gameBoard[state->blankIndex] != 0) {
        LOG("The blankIndex %d in the state that I'm evaluating doesn't point to a blank.\n", state->blankIndex);
        LOG_GAME();
        exit(EXIT_FAILURE);
    }
#endif
    if (isFinal(state)) {
        LOG("This state is final.\n");
        saveResultIfBetter(state);
    } else if (makesSenseToGoDeeper(state)) {
        LOG("Makes sense to go deeper, pushing followup states.\n");
        pushFollowupStates(state);
    }
    previousState = state;
}

void copyGameBoard(int *src, int *target)
{
    int i;
    for (i = 0; i < gameBoardFieldCount; i++) {
        target[i] = src[i];
    }
}
