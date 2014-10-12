//
//  followup.c
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#include "followup.h"
#include <stdlib.h>
#include "mapping.h"

extern int gameBoardRows;

#ifdef DEBUG
extern int allocatedStates;
#endif

State *prepareFollowupState(State *state)
{
    State* prepared = (State *)malloc(sizeof(State));
#ifdef DEBUG
    allocatedStates++;
#endif
    prepared->parent = state;
    prepared->depth = state->depth + 1;
    return prepared;
}

void pushLeftAndTopLeft(State* state, int row, int rowStart, int rowEnd)
{
    int blankIndex = state->blankIndex - 1;
    if (!state->parent || blankIndex != state->parent->blankIndex) {
        State* left = prepareFollowupState(state);
        left->blankIndex = blankIndex;
        pushState(left);
    }
    blankIndex = state->blankIndex - (rowEnd - rowStart + 1);
    if (!state->parent || blankIndex != state->parent->blankIndex) {
        State* topLeft = prepareFollowupState(state);
        topLeft->blankIndex = blankIndex;
        pushState(topLeft);
    }
}

void pushRightAndTopRight(State* state, int row, int rowStart, int rowEnd)
{
    int blankIndex = state->blankIndex + 1;
    if (!state->parent || blankIndex != state->parent->blankIndex) {
        State *right = prepareFollowupState(state);
        right->blankIndex = blankIndex;
        pushState(right);
    }
    blankIndex = state->blankIndex - (rowEnd - rowStart);
    if (!state->parent || blankIndex != state->parent->blankIndex) {
        State *topRight = prepareFollowupState(state);
        topRight->blankIndex = blankIndex;
        pushState(topRight);
    }
}

void pushBottomLeftAndBottomRight(State* state, int row, int rowStart, int rowEnd)
{
    int blankIndex = state->blankIndex + (rowEnd - rowStart + 1);
    if (!state->parent || blankIndex != state->parent->blankIndex) {
        State *bottomLeft = prepareFollowupState(state);
        bottomLeft->blankIndex = blankIndex;
        pushState(bottomLeft);
    }
    blankIndex = state->blankIndex + (rowEnd - rowStart + 2);
    if (!state->parent || blankIndex != state->parent->blankIndex) {
        State *bottomRight = prepareFollowupState(state);
        bottomRight->blankIndex = blankIndex;
        pushState(bottomRight);
    }
}

void pushFollowupStates(State* state)
{
    int row = rowsFromFieldCount(state->blankIndex);
    int rowStart = fieldCountFromRows(row);
    int rowEnd = fieldCountFromRows(row + 1) - 1;
    if (state->blankIndex > rowStart) {
        pushLeftAndTopLeft(state, row, rowStart, rowEnd);
    }
    if (state->blankIndex < rowEnd) {
        pushRightAndTopRight(state, row, rowStart, rowEnd);
    }
    if (row < gameBoardRows - 1) {
        pushBottomLeftAndBottomRight(state, row, rowStart, rowEnd);
    }
}
