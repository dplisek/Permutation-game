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
#include "action.h"

int *gameBoard;
int gameBoardFieldCount;
int gameBoardRows;
int maxDepth;

int expandCycles = 0;
char job;
int minDepth = INT_MAX;
int *resultSteps = NULL;

int processNum;
int donorProcessNum;
int totalProcesses;
StateStack* stateStack = NULL;
State *initialState, *previousState = NULL;

void doInitialize()
{
    evaluateNextStackState();
    if (stateStack->size >= totalProcesses) {
        handOutStatesToAllProcesses();
        job = JOB_EXPANDING;
    }
}

void doExpand()
{
    int flag;
    MPI_Status status;
    evaluateNextStackState();
    if (!stateStack->size) {
        job = JOB_WAITING_FOR_WORK;
    }
    if (expandCycles++ == MSG_CHECK_FREQ) {
        expandCycles = 0;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        if (flag) {
            evaluateMessageWithTagFromSource(status.MPI_TAG, status.MPI_SOURCE);
        }
    }
}

void doWaitForInitialWork()
{
    receiveWorkFromSource(0);
    job = JOB_EXPANDING;
}

int main(int argc, char * argv[])
{
    BOOL done = NO;
    
    MPI_Init(&argc, &argv);
    initProcessNums();
    INIT_LOG();
    LOG("My process: %d, total processes: %d, initial donor process: %d.\n", processNum, totalProcesses, donorProcessNum);
    if (processNum == 0) {
        loadInputData(argc, argv);
        pushInitialState();
        job = JOB_INITIALIZING;
    } else {
        job = JOB_WAITING_FOR_INITIAL_WORK;
    }
    
    while (!done) {
        switch (job) {
            case JOB_INITIALIZING:
                doInitialize();
            case JOB_EXPANDING:
                doExpand();
                break;
            case JOB_WAITING_FOR_INITIAL_WORK:
                doWaitForInitialWork();
                break;
            default:
                break;
        }
    }
    
    
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
