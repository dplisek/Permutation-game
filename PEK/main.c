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

// input
int *gameBoard;
int gameBoardFieldCount;
int gameBoardRows;
int maxDepth;

// results
int minDepth = INT_MAX;
int *resultSteps = NULL;

// processes
int processNum;
int donorProcessNum;
int totalProcesses;

// runtime
int expandCycles = 0;
BOOL done = NO;

// state stack
StateStack* stateStack = NULL;
State *initialState, *previousState = NULL;

int main(int argc, char * argv[])
{
    int flag;
    MPI_Status status;
    
    MPI_Init(&argc, &argv);
    initProcessNums();
    INIT_LOG();
    LOG("My process: %d, total processes: %d, initial donor process: %d.\n", processNum, totalProcesses, donorProcessNum);
    initStateStack();
    if (processNum == 0) {
        LOG("I am the main process, going to load input data.\n");
        loadInputData(argc, argv);
        pushInitialState();
        LOG("Pushed initial state, will begin handing out work.\n");
        initialize();
    }
    
    while (!done) {
        LOG("Starting cycle.\n");
        if (stateStack->size) {
            evaluateNextStackState();
            expandCycles++;
        }
        if (!stateStack->size || expandCycles == MSG_CHECK_FREQ) {
            expandCycles = 0;
            LOG("Checking for incoming message.\n");
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if (flag) {
                LOG("Received a message.\n");
                switch (status.MPI_TAG) {
                    case TAG_WORK_REQUEST:
                        handleWorkRequestFrom(status.MPI_SOURCE);
                        break;
                    case TAG_WORK_RESPONSE:
                        handleWorkResponseFrom(status.MPI_SOURCE);
                        break;
                    case TAG_WORK_NOWORK:
                        handleNoWorkFrom(status.MPI_SOURCE);
                        break;
                    case TAG_TOKEN:
                        handleTokenFrom(status.MPI_SOURCE);
                        break;
                    case TAG_FINISH:
                        handleFinishFrom(status.MPI_SOURCE);
                        break;
                    case TAG_INITIAL_DATA:
                        handleInitialDataFrom(status.MPI_SOURCE);
                        break;
                }
            }
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
