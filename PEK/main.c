/*
 main.c
 PEK
 
 Created by Dominik Plisek on 28/09/14.
 Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "logging.h"
#include "communication.h"
#include "startup.h"
#include "config.h"

int *gameBoard;
int gameBoardFieldCount;
int gameBoardRows;
int maxDepth;

int minDepth = INT_MAX;
int *resultSteps = NULL;

int processNum;
int donorProcessNum;
int totalProcesses;
StateStack* stateStack = NULL;
State *initialState, *previousState = NULL;

int main(int argc, char * argv[])
{
    MPI_Init(&argc, &argv);
    initProcessNums();
    INIT_LOG();
    LOG("My process: %d, total processes: %d, initial donor process: %d.\n", processNum, totalProcesses, donorProcessNum);
    if (processNum == 0) {
        if (!prepareOperation(argc, argv)) return EXIT_FAILURE;
    } else {
        if (!receiveWork()) return EXIT_FAILURE;
    }
//    while (stateStack->size) {
//        evaluateNextStackState();
//    }
//    resetGameBoardFromLastState(previousState);
//    if (resultSteps) {
//        if (!writeResultToFile(argv[3], initialState)) {
//            printf("Couldn't write to file %s.\n", argv[3]);
//            return EXIT_FAILURE;
//        }
//        printf("Analysis complete. The shortest solution has %d steps and has been saved to %s.\n", minDepth, argv[3]);
//        free(resultSteps);
//    } else {
//        printf("Could not find any solution with at most %d steps.\n", maxDepth);
//    }
    if (initialState) free(initialState);
    if (stateStack) freeStateStack();
    if (gameBoard) free(gameBoard);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
