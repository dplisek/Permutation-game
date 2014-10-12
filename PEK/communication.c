//
//  communication.c
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#include "communication.h"
#include "config.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include "action.h"

#define MPI_BOOL MPI_CHAR
#define MPI_COLOR MPI_CHAR

char transferBuffer[TRANSFER_BUFFER_LEN];

extern StateStack *stateStack;
extern int processNum;
extern int job;
extern int totalProcesses, donorProcessNum;
extern BOOL done;
extern BOOL waitingForWork;
extern State *previousState;

extern BOOL hasToken;
extern COLOR color, tokenColor;

extern int *gameBoard;
extern int *initialGameBoard;
extern int gameBoardFieldCount;
extern int gameBoardRows;
extern int maxDepth;

#ifdef DEBUG
extern int allocatedStates;
#endif

void sendInitialDataToProcess(int process)
{
    int position = 0;
    MPI_Pack(&gameBoardFieldCount, 1, MPI_INT, transferBuffer, TRANSFER_BUFFER_LEN, &position, MPI_COMM_WORLD);
    MPI_Pack(initialGameBoard, gameBoardFieldCount, MPI_INT, transferBuffer, TRANSFER_BUFFER_LEN, &position, MPI_COMM_WORLD);
    MPI_Pack(&gameBoardRows, 1, MPI_INT, transferBuffer, TRANSFER_BUFFER_LEN, &position, MPI_COMM_WORLD);
    MPI_Pack(&maxDepth, 1, MPI_INT, transferBuffer, TRANSFER_BUFFER_LEN, &position, MPI_COMM_WORLD);
    MPI_Send(transferBuffer, position, MPI_PACKED, process, TAG_INITIAL_DATA, MPI_COMM_WORLD);
}

void sendStatesWithCommonParentToProcess(State **states, int stateCount, int process)
{
    int historyLength, i = 0, position = 0;
    State *historyItem;
    if (!stateCount) return;
    historyLength = states[0]->depth;
    LOG("Packing history length: %d\n", historyLength);
    MPI_Pack(&(historyLength), 1, MPI_INT, transferBuffer, TRANSFER_BUFFER_LEN, &position, MPI_COMM_WORLD);
    LOG("Packing history.\n");
    historyItem = states[0]->parent;
    while (historyItem) {
        LOG("Packing history item #%d with blankIndex %d.\n", i++, historyItem->blankIndex);
        MPI_Pack(&(historyItem->blankIndex), 1, MPI_INT, transferBuffer, TRANSFER_BUFFER_LEN, &position, MPI_COMM_WORLD);
        historyItem = historyItem->parent;
    }
    LOG("Packing state count: %d\n", stateCount);
    MPI_Pack(&stateCount, 1, MPI_INT, transferBuffer, TRANSFER_BUFFER_LEN, &position, MPI_COMM_WORLD);
    LOG("Packing states.\n");
    for (i = 0; i < stateCount; i++) {
#ifdef DEBUG
        if (states[i]->parent != states[0]->parent) {
            fprintf(stderr, "Trying to send states not with a common parent.\n");
            exit(EXIT_FAILURE);
        }
#endif
        LOG("Packing state #%d with blankIndex %d.\n", i, states[i]->blankIndex);
        MPI_Pack(&(states[i]->blankIndex), 1, MPI_INT, transferBuffer, TRANSFER_BUFFER_LEN, &position, MPI_COMM_WORLD);
        free(states[i]);
#ifdef DEBUG
        allocatedStates--;
#endif
    }
    LOG("Sending package.\n");
    MPI_Send(transferBuffer, position, MPI_PACKED, process, TAG_WORK_RESPONSE, MPI_COMM_WORLD);
    LOG("Package sent.\n");
}

void requestWork()
{
    LOG("Requesting work from process %d.\n", donorProcessNum);
    MPI_Send(NULL, 0, MPI_INT, donorProcessNum, TAG_WORK_REQUEST, MPI_COMM_WORLD);
    donorProcessNum = (donorProcessNum + 1) % totalProcesses;
    if (donorProcessNum == processNum) {
        donorProcessNum = (donorProcessNum + 1) % totalProcesses;
    }
}

void handOutInitialDataToAllProcesses()
{
    int i, dest;
    for (i = 1; i < totalProcesses; i++) {
        dest = (processNum + i) % totalProcesses;
        LOG("Sending initial data to process %d.\n", dest);
        sendInitialDataToProcess(dest);
        State *state = popState();
        LOG("Sending state with depth %d and blankIndex %d and parent blankIndex %d to process %d.\n", state->depth, state->blankIndex, state->parent ? state->parent->blankIndex : -1, dest);
        sendStatesWithCommonParentToProcess(&state, 1, dest);
    }
}

void sendTokenTo(int dest, COLOR tokenColor)
{
    MPI_Send(&tokenColor, 1, MPI_BOOL, dest, TAG_TOKEN, MPI_COMM_WORLD);
    LOG("Was %s. Sent a %s token.\n", color ? "black" : "white", tokenColor ? "black" : "white");
    hasToken = NO;
    color = WHITE;
    LOG("Became white after sending token on.\n");
}

void broadcastFinish()
{
    int i, dest;
    for (i = 1; i < totalProcesses; i++) {
        dest = (processNum + i) % totalProcesses;
        LOG("Sending finish to process %d.\n", dest);
        MPI_Send(NULL, 0, MPI_INT, dest, TAG_FINISH, MPI_COMM_WORLD);
    }
}

void handleWorkRequestFrom(int source)
{
    MPI_Status status;
    LOG("Received work request from process %d.\n", source);
    MPI_Recv(NULL, 0, MPI_INT, source, TAG_WORK_REQUEST, MPI_COMM_WORLD, &status);
    if (stateStack->size) {
        LOG("Have work to give.\n");
        State *state = popStateOffBottom();
        LOG("Deepest state has blankIndex %d and depth %d and parent blankIndex %d. Will give it away.\n", state->blankIndex, state->depth, state->parent ? state->parent->blankIndex : -1);
        sendStatesWithCommonParentToProcess(&state, 1, source);
        if (source < processNum) {
            color = BLACK;
            LOG("Now have a bad conscience, set color to black.\n");
        }
    } else {
        LOG("Don't have work, sending nowork.\n");
        MPI_Send(NULL, 0, MPI_INT, source, TAG_WORK_NOWORK, MPI_COMM_WORLD);
    }
}

void handleWorkResponseFrom(int source)
{
    int i, blankIndex, stateCount, position = 0, historyLength = 0;
    State *state, *firstState = NULL, *childState = NULL;
    MPI_Status status;
#ifdef DEBUG
    if (stateStack && stateStack->size) {
        fprintf(stderr, "Received work without an empty stack, that should never happen.\n");
        exit(EXIT_FAILURE);
    }
#endif
    MPI_Recv(transferBuffer, TRANSFER_BUFFER_LEN, MPI_PACKED, source, TAG_WORK_RESPONSE, MPI_COMM_WORLD, &status);
    LOG("Received work.\n");
    MPI_Unpack(transferBuffer, TRANSFER_BUFFER_LEN, &position, &historyLength, 1, MPI_INT, MPI_COMM_WORLD);
    LOG("Unpacked history length: %d\n", historyLength);
#ifdef DEBUG
    if (!historyLength) {
        fprintf(stderr, "Received states without a parent.\n");
        exit(EXIT_FAILURE);
    }
#endif
    for (i = historyLength - 1; i >= 0; i--) {
        MPI_Unpack(transferBuffer, TRANSFER_BUFFER_LEN, &position, &blankIndex, 1, MPI_INT, MPI_COMM_WORLD);
        LOG("Unpacked history item #%d with blankIndex %d.\n", i, blankIndex);
        state = (State *)malloc(sizeof(State));
#ifdef DEBUG
        allocatedStates++;
#endif
        state->blankIndex = blankIndex;
        state->depth = i;
        state->parent = NULL;
        if (childState) {
            childState->parent = state;
            LOG("State with blankIndex %d set as parent of state with blankIndex %d.\n", state->blankIndex, childState->blankIndex);
        } else {
            firstState = state;
            LOG("State with blankIndex %d set as first state, will become parent of received work states.\n", state->blankIndex);
        }
        childState = state;
    }
    MPI_Unpack(transferBuffer, TRANSFER_BUFFER_LEN, &position, &stateCount, 1, MPI_INT, MPI_COMM_WORLD);
    LOG("Unpacked state count: %d\n", stateCount);
    for (i = 0; i < stateCount; i++) {
        MPI_Unpack(transferBuffer, TRANSFER_BUFFER_LEN, &position, &blankIndex, 1, MPI_INT, MPI_COMM_WORLD);
        LOG("Unpacked work state #%d with blankIndex %d. Will push to stack.\n", i, blankIndex);
        state = (State *)malloc(sizeof(State));
#ifdef DEBUG
        allocatedStates++;
#endif
        state->blankIndex = blankIndex;
        state->depth = firstState->depth + 1;
        state->parent = firstState;
        pushState(state);
    }
    LOG("Finished receiving work, saved %d states to the stack.\n", stateCount);
    forwardToState(firstState);
    previousState = firstState;
    LOG("Forwarded to parent state of received work states.\n");
    waitingForWork = NO;
}


void handleNoWorkFrom(int source)
{
    MPI_Status status;
    MPI_Recv(NULL, 0, MPI_INT, source, TAG_WORK_NOWORK, MPI_COMM_WORLD, &status);
    LOG("Source %d has no work. Going to ask next in line.\n", source);
    requestWork();
}

void handleTokenFrom(int source)
{
    MPI_Status status;
    MPI_Recv(&tokenColor, 1, MPI_COLOR, source, TAG_TOKEN, MPI_COMM_WORLD, &status);
    LOG("Received a %s token.\n", tokenColor ? "black" : "white");
    hasToken = YES;
    if (processNum == 0) {
        if (tokenColor == WHITE) {
            printf("Token detected finish.\n");
            broadcastFinish();
            done = YES;
        } else {
            tokenColor = WHITE;
        }
    }
}

void handleFinishFrom(int source)
{
    MPI_Status status;
    MPI_Recv(NULL, 0, MPI_INT, source, TAG_FINISH, MPI_COMM_WORLD, &status);
    LOG("Received finish message, setting done to YES.\n");
    done = YES;
}

void handleInitialDataFrom(int source)
{
    int position = 0;
    MPI_Status status;
    MPI_Recv(transferBuffer, TRANSFER_BUFFER_LEN, MPI_PACKED, source, TAG_INITIAL_DATA, MPI_COMM_WORLD, &status);
    MPI_Unpack(transferBuffer, TRANSFER_BUFFER_LEN, &position, &gameBoardFieldCount, 1, MPI_INT, MPI_COMM_WORLD);
    gameBoard = malloc(sizeof(int) * gameBoardFieldCount);
    MPI_Unpack(transferBuffer, TRANSFER_BUFFER_LEN, &position, gameBoard, gameBoardFieldCount, MPI_INT, MPI_COMM_WORLD);
    initialGameBoard = malloc(sizeof(int) * gameBoardFieldCount);
    copyGameBoard(gameBoard, initialGameBoard);
    MPI_Unpack(transferBuffer, TRANSFER_BUFFER_LEN, &position, &gameBoardRows, 1, MPI_INT, MPI_COMM_WORLD);
    MPI_Unpack(transferBuffer, TRANSFER_BUFFER_LEN, &position, &maxDepth, 1, MPI_INT, MPI_COMM_WORLD);
    LOG("Received initial data, game board rows: %d, max depth: %d.\n", gameBoardRows, maxDepth);
}
