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

#define GAME_BOARD_INITIAL_ALLOC 10

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
    int value;
    fscanf(file, "%u", &value);
    while (!feof(file)) {
        gameBoardFieldCount++;
        if (gameBoardFieldCount > gameBoardFieldCountAllocated) {
            gameBoardFieldCountAllocated *= gameBoardFieldCountAllocated;
            gameBoard = realloc(gameBoard, sizeof(int) * gameBoardFieldCountAllocated);
        }
        gameBoard[gameBoardFieldCount - 1] = value;
        fscanf(file, "%u", &value);
    }
    fclose(file);
    gameBoardRows = rowsFromFieldCount(gameBoardFieldCount);
    printGameBoard();
    return YES;
}

void pushState(State *state)
{
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
    if (state->blankIndex > rowStart) {
        State* left = prepareFollowupState(state);
        left->blankIndex = state->blankIndex - 1;
        pushState(left);
        State* topLeft = prepareFollowupState(state);
        topLeft->blankIndex = state->blankIndex - (rowEnd - rowStart + 1);
        pushState(topLeft);
    }
    if (state->blankIndex < rowEnd) {
        State *right = prepareFollowupState(state);
        right->blankIndex = state->blankIndex + 1;
        pushState(right);
        State *topRight = prepareFollowupState(state);
        topRight->blankIndex = state->blankIndex - (rowEnd - rowStart);
        pushState(topRight);
    }
    if (row < gameBoardRows - 1) {
        
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
    State *initialState = findInitialState();
    if (!initialState) return EXIT_FAILURE;
    pushState(initialState);
    State *previousState = NULL;
    while (stateStack->size) {
        State *state = popState();
        if (state->parent) {
            if (previousState) {
                while (previousState != state->parent) {
                    swapIndices(previousState->blankIndex, previousState->parent->blankIndex);
                    previousState = previousState->parent;
                    if (!previousState) {
                        printf("Reached top of state space while searching for common parent.\n");
                        return EXIT_FAILURE;
                    }
                }
            }
            swapIndices(state->parent->blankIndex, state->blankIndex);
        }
        if (isFinal(state)) {
            if (state->depth < minDepth) {
                minDepth = state->depth;
                saveResult(state);
            }
            continue;
        }
        if (state->depth == maxDepth || state->depth == minDepth - 1) continue;
        pushFollowupStates(state);
        previousState = state;
    }
    return EXIT_SUCCESS;
}
