//
//  main.c
//  PEK
//
//  Created by Dominik Plisek on 28/09/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>

#define GAME_BOARD_INITIAL_ALLOC 10
#define STACK_INITIAL_ALLOC 10

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

StateStack* result;
int minDepth = INT_MAX;

#pragma mark - Runtime

StateStack* stateStack;

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

void printGameBoard()
{
    printf("Fields: %u, rows: %u\n", gameBoardFieldCount, gameBoardRows);
    int index = 0;
    for (int row = 0; row < gameBoardRows; row++) {
        for (int col = 0; col <= row; col++) {
            printf("%u\t", gameBoard[index]);
            index++;
        }
        printf("\n");
    }
}

void Log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf(fmt, args);
    printf("\n\n");
    printGameBoard();
    printf("\n\n");
    printf("-------------------------------------------------------------------");
    printf("\n\n");
    va_end(args);
}

#pragma mark - Startup

BOOL loadGameBoardFromFileName(const char *fileName)
{
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        printf("Input file with name %s not found.\n", fileName);
        return NO;
    }
    int gameBoardFieldCountAllocated = GAME_BOARD_INITIAL_ALLOC;
    gameBoard = (int *)malloc(sizeof(int) * gameBoardFieldCountAllocated);
    int value;
    fscanf(file, "%u", &value);
    while (!feof(file)) {
        if (gameBoardFieldCount + 1 > gameBoardFieldCountAllocated) {
            int newAlloc = gameBoardFieldCountAllocated * gameBoardFieldCountAllocated;
            gameBoard = realloc(gameBoard, sizeof(int) * newAlloc);
            gameBoardFieldCountAllocated = newAlloc;
        }
        gameBoard[gameBoardFieldCount] = value;
        gameBoardFieldCount++;
        fscanf(file, "%u", &value);
    }
    fclose(file);
    gameBoardRows = rowsFromFieldCount(gameBoardFieldCount);
    return YES;
}

State *findInitialState()
{
    for (int i = 0; i < gameBoardFieldCount; i++) {
        int value = gameBoard[i];
        if (value == 0) {
            State *state = (State *)malloc(sizeof(State));
            state->depth = 0;
            state->blankIndex = i;
            return state;
        }
    }
    printf("Missing \"0\" on game board. Please check input.\n");
    return NULL;
}

#pragma mark - State stack

void pushState(State *state)
{
    if (!stateStack) {
        stateStack = (StateStack *)malloc(sizeof(StateStack));
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

#pragma mark - Action

void swapIndices(int i1, int i2)
{
    int temp = gameBoard[i1];
    gameBoard[i1] = gameBoard[i2];
    gameBoard[i2] = temp;
}

BOOL backUpAndFindCommonParent(State *state, State *previousState)
{
    while (previousState != state->parent) {
        if (!previousState->parent) {
            printf("Reached top of state space while searching for common parent. That is an error.\n");
            return NO;
        }
        swapIndices(previousState->blankIndex, previousState->parent->blankIndex);
        previousState = previousState->parent;
    }
    return YES;
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

#pragma mark - Results

BOOL isFinal(State *state)
{
    for (int i = 0; i < gameBoardFieldCount; i++) {
        if (i != gameBoard[i]) return NO;
    }
    return YES;
}

BOOL saveResult(State *state, const char *fileName)
{
    FILE *file = fopen(fileName, "w");
    if (file == NULL) {
        printf("Output file with name %s not found.\n", fileName);
        return NO;
    }
    
    return YES;
}

BOOL saveResultIfBetter(State *state, const char *fileName)
{
    if (state->depth < minDepth) {
        minDepth = state->depth;
        return saveResult(state, fileName);
    }
    return YES;
}

#pragma mark - Support functions

BOOL makesSenseToGoDeeper(State *state)
{
    return state->depth < maxDepth && state->depth < minDepth - 1;
}

#pragma mark - Main

int main(int argc, const char * argv[])
{
    if (argc < 4) {
        printf("Usage: programname <gameboard file> <max depth> <output file>\n");
        return EXIT_FAILURE;
    }
    const char *fileName = argv[1];
    if (!loadGameBoardFromFileName(fileName)) return EXIT_FAILURE;
    Log("Board loaded.");
    maxDepth = atoi(argv[2]);
    printf("Input max depth is %d.\n", maxDepth);
    State *initialState = findInitialState();
    if (!initialState) return EXIT_FAILURE;
    pushState(initialState);
    State *previousState = NULL;
    while (stateStack->size) {
        State *state = popState();
        if (state->parent) {
            if (!backUpAndFindCommonParent(state, previousState)) return EXIT_FAILURE;
            swapIndices(state->parent->blankIndex, state->blankIndex);
        }
        previousState = state;
        if (isFinal(state)) {
            saveResultIfBetter(state, argv[3]);
            continue;
        }
        if (makesSenseToGoDeeper(state)) {
            pushFollowupStates(state);
        }
    }
    printf("Program complete. The shortest solution has %d steps and has been saved to %s.\n", minDepth, argv[3]);
    return EXIT_SUCCESS;
}
