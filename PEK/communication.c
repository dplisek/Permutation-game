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

#define TAG_WORK_REQUEST 100
#define TAG_WORK_RESPONSE 101

char transferBuffer[TRANSFER_BUFFER_LEN];
extern StateStack *stateStack;
extern int processNum;
extern int job;
extern int totalProcesses;

BOOL sendStatesWithCommonParentToProcess(State **states, int stateCount, int process)
{
    int historyLength, i = 0, position = 0;
    State *historyItem;
    if (!stateCount) return NO;
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
        if (states[i]->depth != historyLength) {
            fprintf(stderr, "Trying to send states not with a common parent.\n");
            return NO;
        }
#endif
        LOG("Packing state #%d with blankIndex %d.\n", i, states[i]->blankIndex);
        MPI_Pack(&(states[i]->blankIndex), 1, MPI_INT, transferBuffer, TRANSFER_BUFFER_LEN, &position, MPI_COMM_WORLD);
    }
    LOG("Sending package.\n");
    MPI_Send(transferBuffer, position, MPI_PACKED, process, TAG_WORK_RESPONSE, MPI_COMM_WORLD);
    LOG("Package sent.\n");
    return YES;
}

void requestWorkFrom(int dest)
{
    MPI_Send(&processNum, 1, MPI_INT, dest, TAG_WORK_REQUEST, MPI_COMM_WORLD);
}

BOOL receiveWorkFromSource(int source)
{
    int i, blankIndex, stateCount, position = 0, historyLength = 0;
    State *state, *firstState = NULL, *childState = NULL;
    MPI_Status status;
#ifdef DEBUG
    if (stateStack && stateStack->size) {
        fprintf(stderr, "Asking for work without an empty stack, that should never happen.\n");
        return NO;
    }
#endif
    MPI_Recv(transferBuffer, TRANSFER_BUFFER_LEN, MPI_PACKED, source, TAG_WORK_RESPONSE, MPI_COMM_WORLD, &status);
    LOG("Received work.\n");
    MPI_Unpack(transferBuffer, TRANSFER_BUFFER_LEN, &position, &historyLength, 1, MPI_INT, MPI_COMM_WORLD);
    LOG("Unpacked history length: %d\n", historyLength);
#ifdef DEBUG
    if (!historyLength) {
        fprintf(stderr, "Received states without a parent.\n");
        return NO;
    }
#endif
    for (i = historyLength - 1; i >= 0; i--) {
        MPI_Unpack(transferBuffer, TRANSFER_BUFFER_LEN, &position, &blankIndex, 1, MPI_INT, MPI_COMM_WORLD);
        LOG("Unpacked history item #%d with blankIndex %d.\n", i, blankIndex);
        state = (State *)malloc(sizeof(State));
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
        state->blankIndex = blankIndex;
        state->depth = firstState->depth + 1;
        state->parent = firstState;
        pushState(state);
    }
    LOG("Finished receiving work, saved %d states to the stack.\n", stateCount);
    return YES;
}

void evaluateMessageWithTagFromSource(int tag, int source)
{
    switch (tag) {
        case TAG_WORK_REQUEST:

            break;
        case TAG_WORK_RESPONSE:
            receiveWorkFromSource(source);
            job = JOB_EXPANDING;
            break;
        default:
            break;
    }
}

void handOutStatesToAllProcesses()
{
    int i;
    for (i = 1; i < totalProcesses; i++) {
        State *state = popState();
        sendStatesWithCommonParentToProcess(&state, 1, i);
    }
}

