//
//  startup.c
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#include "startup.h"
#include "communication.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include "mapping.h"
#include "logging.h"
#include "action.h"
#include "output.h"

extern int *gameBoard;
extern int gameBoardFieldCount;
extern int gameBoardRows;
extern int processNum, totalProcesses, donorProcessNum;
extern int maxDepth;
extern State *initialState;
extern StateStack *stateStack;

void initProcessNums()
{
    MPI_Comm_rank(MPI_COMM_WORLD, &processNum);
    MPI_Comm_size(MPI_COMM_WORLD, &totalProcesses);
    donorProcessNum = (processNum + 1) % totalProcesses;
}

BOOL loadGameBoardFromFileName(const char *fileName)
{
    int gameBoardFieldCountAllocated = GAME_BOARD_INITIAL_ALLOC;
    int value;
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        printf("Input file with name %s not found.\n", fileName);
        return NO;
    }
    gameBoard = (int *)malloc(sizeof(int) * gameBoardFieldCountAllocated);
    fscanf(file, "%d", &value);
    while (!feof(file)) {
        if (gameBoardFieldCount + 1 > gameBoardFieldCountAllocated) {
            int newAlloc = gameBoardFieldCountAllocated * gameBoardFieldCountAllocated;
            gameBoard = (int *)realloc(gameBoard, sizeof(int) * newAlloc);
            gameBoardFieldCountAllocated = newAlloc;
        }
        gameBoard[gameBoardFieldCount] = value;
        gameBoardFieldCount++;
        fscanf(file, "%d", &value);
    }
    fclose(file);
    gameBoardRows = rowsFromFieldCount(gameBoardFieldCount);
    return YES;
}

State *findInitialState()
{
    int i;
    for (i = 0; i < gameBoardFieldCount; i++) {
        int value = gameBoard[i];
        if (value == 0) {
            State *state = (State *)malloc(sizeof(State));
            state->depth = 0;
            state->blankIndex = i;
            state->parent = NULL;
            return state;
        }
    }
    printf("Missing \"0\" on game board. Please check input.\n");
    return NULL;
}

BOOL prepareOperation(int argc, char * argv[])
{
    int i, toDistribute;
    const char *fileName;
    if (argc < 4) {
        fprintf(stderr, "Usage: program-name <gameboard file> <max depth> <output file>\n");
        return NO;
    }
    fileName = argv[1];
    if (!loadGameBoardFromFileName(fileName)) return NO;
    printf("Board loaded. Fields: %d, rows: %d\n", gameBoardFieldCount, gameBoardRows);
    printGameBoardToStream(stdout);
    maxDepth = atoi(argv[2]);
    printf("Input max depth is %d.\n", maxDepth);
    initialState = findInitialState();
    if (!initialState) return NO;
    pushState(initialState);
    evaluateNextStackState();
    if (totalProcesses > 1) {
        toDistribute = stateStack->size;
        LOG("Going to distribute %d initial states.\n", toDistribute);
        for (i = 0; i < toDistribute; i++) {
            int dest = (i + 1) % totalProcesses;
            if (dest != 0) {
                State *state = popState();
                LOG("Distributing state %d with blankIndex %d to destination process %d.\n", i, state->blankIndex, dest);
                if (!sendStatesWithCommonParentToProcess(&state, 1, dest)) return NO;
            } else {
                LOG("Keeping state %d.\n", i);
            }
        }
    }
    return YES;
}
