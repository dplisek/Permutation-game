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

void initProcessNums()
{
    MPI_Comm_rank(MPI_COMM_WORLD, &processNum);
    MPI_Comm_size(MPI_COMM_WORLD, &totalProcesses);
    donorProcessNum = (processNum + 1) % totalProcesses;
}

void loadGameBoardFromFileName(const char *fileName)
{
    int gameBoardFieldCountAllocated = GAME_BOARD_INITIAL_ALLOC;
    int value;
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        printf("Input file with name %s not found.\n", fileName);
        exit(EXIT_FAILURE);
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
}

void loadInputData(int argc, char *argv[])
{
    const char *fileName;
    if (argc < 4) {
        fprintf(stderr, "Usage: program-name <gameboard file> <max depth> <output file>\n");
        exit(EXIT_FAILURE);
    }
    fileName = argv[1];
    loadGameBoardFromFileName(fileName);
    printf("Board loaded. Fields: %d, rows: %d\n", gameBoardFieldCount, gameBoardRows);
    printGameBoardToStream(stdout);
    maxDepth = atoi(argv[2]);
    printf("Input max depth is %d.\n", maxDepth);
}

void pushInitialState()
{
    int i;
    for (i = 0; i < gameBoardFieldCount; i++) {
        int value = gameBoard[i];
        if (value == 0) {
            initialState = (State *)malloc(sizeof(State));
            initialState->depth = 0;
            initialState->blankIndex = i;
            initialState->parent = NULL;
            pushState(initialState);
            return;
        }
    }
    printf("Missing \"0\" on game board. Please check input.\n");
    exit(EXIT_FAILURE);
}
