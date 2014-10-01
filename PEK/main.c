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

#pragma mark Input

int *gameBoard;
int gameBoardFieldCount;
int gameBoardRows;
int maxDepth;

#pragma mark Output

StateStack* result;
int minDepth = INT_MAX;

#pragma mark Runtime

StateStack* stateStack;

#pragma mark - Support functions

int fieldCountFromRows(int rows)
{
    return rows * (rows + 1) / 2;
}

int rowsFromFieldCount(int fieldCount)
{
    return (-1 + sqrt(1 + (8 * fieldCount))) / 2;
}

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

BOOL loadGameBoardFromFileName(const char *fileName)
{
    FILE *file = fopen(fileName, "r");
    if (file == NULL) {
        printf("File with name %s not found.\n", fileName);
        return NO;
    }
    int gameBoardFieldCountAllocated = GAME_BOARD_INITIAL_ALLOC;
    gameBoard = (int *)malloc(sizeof(int) * gameBoardFieldCountAllocated);
    printf("Initially allocated %d ints for game board fields.\n", gameBoardFieldCountAllocated);
    int value;
    fscanf(file, "%u", &value);
    while (!feof(file)) {
        if (gameBoardFieldCount + 1 > gameBoardFieldCountAllocated) {
            int newAlloc = gameBoardFieldCountAllocated * gameBoardFieldCountAllocated;
            gameBoard = realloc(gameBoard, sizeof(int) * newAlloc);
            gameBoardFieldCountAllocated = newAlloc;
            printf("Reallocated %d ints for game board fields.\n", gameBoardFieldCountAllocated);
        }
        gameBoard[gameBoardFieldCount] = value;
        gameBoardFieldCount++;
        fscanf(file, "%u", &value);
    }
    fclose(file);
    gameBoardRows = rowsFromFieldCount(gameBoardFieldCount);
    return YES;
}

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

void swapIndices(int i1, int i2)
{
    int temp = gameBoard[i1];
    gameBoard[i1] = gameBoard[i2];
    gameBoard[i2] = temp;
}

BOOL isFinal(State *state)
{
    for (int i = 0; i < gameBoardFieldCount; i++) {
        if (i != gameBoard[i]) return NO;
    }
    return YES;
}

State *prepareFollowupState(State *state)
{
    State* prepared = (State *)malloc(sizeof(State));
    prepared->parent = state;
    prepared->depth = state->depth + 1;
    return prepared;
}

void pushFollowupStates(State* state)
{
    int row = rowsFromFieldCount(state->blankIndex);
    int rowStart = fieldCountFromRows(row);
    int rowEnd = fieldCountFromRows(row + 1) - 1;
    printf("Going to push followup states.\n");
    printf("row: %d, start: %d, end: %d\n", row, rowStart, rowEnd);
    if (state->blankIndex > rowStart) {
        printf("Will push left and top left.\n");
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
    if (state->blankIndex < rowEnd) {
        printf("Will push right and top right.\n");
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
    if (row < gameBoardRows - 1) {
        printf("Will push bottom left and bottom right.\n");
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
}

void saveResult(State *state)
{
    // save result
}

#pragma mark - Main

int main(int argc, const char * argv[])
{
    if (argc < 3) {
        printf("Usage: programname <filename> <max depth>\n");
        return EXIT_FAILURE;
    }
    const char *fileName = argv[1];
    if (!loadGameBoardFromFileName(fileName)) return EXIT_FAILURE;
    Log("Board loaded.");
    maxDepth = atoi(argv[2]);
    printf("Max depth (from input) is %d.\n", maxDepth);
    State *initialState = findInitialState();
    if (!initialState) return EXIT_FAILURE;
    pushState(initialState);
    State *previousState = NULL;
    while (stateStack->size) {
        State *state = popState();
        if (state->parent) {
            printf("State has parent, is not root.\n");
            while (previousState != state->parent) {
                Log("Moved back up a notch.");
                swapIndices(previousState->blankIndex, previousState->parent->blankIndex);
                previousState = previousState->parent;
                if (!previousState) {
                    printf("Reached top of state space while searching for common parent. That is an error.\n");
                    return EXIT_FAILURE;
                }
            }
            swapIndices(state->parent->blankIndex, state->blankIndex);
            Log("Moved to the correct state.");
        }
        if (isFinal(state)) {
            printf("This state is final.\n");
            if (state->depth < minDepth) {
                printf("This state also has a smaller depth (%d) than the current %d, going to save it.\n", state->depth, minDepth);
                minDepth = state->depth;
                saveResult(state);
            }
            continue;
        }
        if (state->depth == maxDepth || state->depth == minDepth - 1) {
            printf("Reached the max depth or the minDepth - 1, no need to go any deeper.\n");
            continue;
        }
        pushFollowupStates(state);
        previousState = state;
    }
    return EXIT_SUCCESS;
}
