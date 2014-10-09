/*
 main.c
 PEK
 
 Created by Dominik Plisek on 28/09/14.
 Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#ifdef STAR
#include "mpi.h"
#else
#include "/usr/local/include/mpi.h"
#endif

#define GAME_BOARD_INITIAL_ALLOC 10
#define STACK_INITIAL_ALLOC 10
#define LOG_FILENAME_FORMAT "logp#%d"
#define MAIN_PROCESS 0

#ifdef DEBUG
#define INIT_LOG() initLog();
#define LOG(args...) logToFile(args);
#else
#define INIT_LOG()
#define LOG(text, ...)
#endif

#define YES 1
#define NO 0

typedef char BOOL;

#pragma mark - Structs

typedef struct state {
    struct state *parent;
    int blankIndex;
    int depth;
} State;

typedef struct {
    State **states;
    int size;
    int allocated;
} StateStack;

#pragma mark - Global variables

#pragma mark - Input

int *gameBoard;
int gameBoardFieldCount;
int gameBoardRows;
int maxDepth;

#pragma mark - Output

int* resultSteps = NULL;
int minDepth = INT_MAX;

#pragma mark - Runtime

char logFileName[256];
int processNum;
int donorProcessNum;
int totalProcesses;
StateStack* stateStack = NULL;
State *initialState, *previousState = NULL;

#pragma mark - Mapping

int fieldCountFromRows(int rows)
{
    return rows * (rows + 1) / 2;
}

int rowsFromFieldCount(int fieldCount)
{
    return (-1 + sqrt(1 + (8 * fieldCount))) / 2;
}

#pragma mark - Logging

#ifdef DEBUG

void initLog()
{
    sprintf(logFileName, LOG_FILENAME_FORMAT, processNum);
    remove(logFileName);
}

void logToFile(const char *format, ...)
{
    va_list arguments;
    FILE *logFile = fopen(logFileName, "a");
    va_start(arguments, format);
    vfprintf(logFile, format, arguments);
    va_end(arguments);
    fclose(logFile);
}

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

#pragma mark - Support functions

BOOL makesSenseToGoDeeper(State *state)
{
    return state->depth < maxDepth && state->depth < minDepth - 1;
}

#pragma mark - State stack

void pushState(State *state)
{
    if (!stateStack) {
        stateStack = (StateStack *)malloc(sizeof(StateStack));
        stateStack->allocated = 0;
        stateStack->size = 0;
    }
    if (!stateStack->allocated) {
        stateStack->states = (State **)malloc(sizeof(State *) * STACK_INITIAL_ALLOC);
        stateStack->allocated = STACK_INITIAL_ALLOC;
    } else if (stateStack->size + 1 > stateStack->allocated) {
        int newAlloc = stateStack->allocated * stateStack->allocated;
        stateStack->states = (State **)realloc(stateStack->states, sizeof(State *) * newAlloc);
        stateStack->allocated = newAlloc;
    }
    stateStack->states[stateStack->size] = state;
    stateStack->size++;
}

State *popState()
{
    State *state = stateStack->states[stateStack->size - 1];
    stateStack->size--;
    return state;
}

State *popStateOffBottom()
{
    int i;
    State *state = stateStack->states[0];
    for (i = 0; i < stateStack->size - 1; i++) {
        stateStack->states[i] = stateStack->states[i + 1];
    }
    stateStack->size--;
    return state;
}

void freeStateStack()
{
    free(stateStack->states);
    free(stateStack);
}

#pragma mark - Followup

State *prepareFollowupState(State *state)
{
    State* prepared = (State *)malloc(sizeof(State));
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

#pragma mark - Communication

void sendStateToProcess(State *state, int process)
{
    // send
}

#pragma mark - Results

void swapIndices(int i1, int i2);

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
        printf("Found a (better) solution, steps: %d\n", minDepth);
        if (resultSteps) free(resultSteps);
        resultSteps = (int *)malloc(sizeof(int) * minDepth);
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
    }
}

BOOL writeResultToFile(const char *fileName, State *initialState)
{
    int i;
    FILE *file = fopen(fileName, "w");
    if (file == NULL) {
        printf("Output file with name %s not found.\n", fileName);
        return NO;
    }
    fprintf(file, "Start:\n");
    printGameBoardToStream(file);
    for (i = -1; i < minDepth - 1; i++) {
        swapIndices(i >= 0 ? resultSteps[i] : initialState->blankIndex, resultSteps[i + 1]);
        fprintf(file, "Step %d:\n", i + 2);
        printGameBoardToStream(file);
    }
    fclose(file);
    return YES;
}

#pragma mark - Action

void swapIndices(int i1, int i2)
{
    int temp = gameBoard[i1];
    gameBoard[i1] = gameBoard[i2];
    gameBoard[i2] = temp;
}

BOOL backUpAndFindCommonParent(State *state, State *previousState)
{
    State *stateToFree;
    while (previousState != state->parent) {
        if (!previousState->parent) {
            printf("Reached top of state space while searching for common parent. That is an error.\n");
            return NO;
        }
        swapIndices(previousState->blankIndex, previousState->parent->blankIndex);
        stateToFree = previousState;
        previousState = previousState->parent;
        free(stateToFree);
    }
    return YES;
}

BOOL evaluateNextStackState()
{
    State *state = popState();
    if (state->parent) {
        if (!backUpAndFindCommonParent(state, previousState)) return NO;
        swapIndices(state->parent->blankIndex, state->blankIndex);
    }
    if (isFinal(state)) {
        saveResultIfBetter(state);
    } else if (makesSenseToGoDeeper(state)) {
        pushFollowupStates(state);
    }
    previousState = state;
    return YES;
}

#pragma mark - Startup

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
        fprintf(stderr, "Usage: programname <gameboard file> <max depth> <output file>\n");
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
            if (dest != MAIN_PROCESS) {
                LOG("Distributing state %d to destination process %d.\n", i, dest);
                sendStateToProcess(popState(), dest);
            } else {
                LOG("Keeping state %d.\n", i);
            }
        }
    }
    return YES;
}

#pragma mark - Main

int main(int argc, char * argv[])
{
    MPI_Init(&argc, &argv);
    initProcessNums();
    INIT_LOG();
    LOG("My process: %d, total processes: %d, initial donor process: %d.\n", processNum, totalProcesses, donorProcessNum);
    if (processNum == MAIN_PROCESS) {
        if (!prepareOperation(argc, argv)) return EXIT_FAILURE;
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
