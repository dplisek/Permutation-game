//
//  output.c
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#include "output.h"
#include <stdlib.h>
#include "action.h"
#include "logging.h"
#include "communication.h"

extern int *gameBoard;
extern int gameBoardFieldCount;
extern int gameBoardRows;
extern int minDepth;
extern int *resultSteps;
extern int processNum;
extern double execTime;

#ifdef DEBUG
extern int allocatedStates;
#endif

void printGameBoardToStream(FILE *stream)
{
    int row, col, index = 0;
    for (row = 0; row < gameBoardRows; row++) {
        for (col = 0; col <= row; col++) {
            fprintf(stream, "%d\t", gameBoard[index]);
            index++;
        }
        fprintf(stream, "\n");
    }
}

BOOL isFinal(State *state)
{
    int i = 0;
    for (i = 0; i < gameBoardFieldCount; i++) {
        if (i != gameBoard[i]) return NO;
    }
    return YES;
}

void saveResultIfBetter(State *state)
{
    int i;
    if (state->depth < minDepth) {
        minDepth = state->depth;
        LOG("Found a (better) solution, steps: %d\n", minDepth);
        printf("[%.4f] Process %d found a (better) solution, steps: %d\n", MPI_Wtime() - execTime, processNum, minDepth);
        if (resultSteps) {
            resultSteps = realloc(resultSteps, sizeof(int) * minDepth);
        } else {
            resultSteps = malloc(sizeof(int) * minDepth);
        }
        for (i = minDepth - 1; i >= 0; i--) {
            resultSteps[i] = state->blankIndex;
            state = state->parent;
        }
    }
}

void resetGameBoardFromLastState(State *state)
{
    State *stateToFree;
    while (state->parent) {
        swapIndices(state->blankIndex, state->parent->blankIndex);
        stateToFree = state;
        state = state->parent;
        free(stateToFree);
#ifdef DEBUG
        allocatedStates--;
#endif
    }
    free(state);
#ifdef DEBUG
    allocatedStates--;
#endif
}

int findInitialBlankIndex()
{
    int i;
    for (i = 0; i < gameBoardFieldCount; i++) {
        if (gameBoard[i] == 0) return i;
    }
    fprintf(stderr, "Couldn't find 0 on game board.\n");
    exit(EXIT_FAILURE);
}

void writeResultToFile(const char *fileName)
{
    int i, initialBlankIndex = findInitialBlankIndex();
    FILE *file = fopen(fileName, "w");
    if (file == NULL) {
        fprintf(stderr, "Output file with name %s not found.\n", fileName);
        exit(EXIT_FAILURE);
    }
    fprintf(file, "Start:\n");
    printGameBoardToStream(file);
    for (i = -1; i < minDepth - 1; i++) {
        swapIndices(i >= 0 ? resultSteps[i] : initialBlankIndex, resultSteps[i + 1]);
        fprintf(file, "Step %d:\n", i + 2);
        printGameBoardToStream(file);
    }
    fclose(file);
}
