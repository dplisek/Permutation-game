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
int maxDepth;

#pragma mark Output

StateStack* result;
int minDepth = INT_MAX;

#pragma mark Runtime

StateStack* stateStack;

#pragma mark - Support functions

int rowsFromFieldCount(int fieldCount)
{
    return (-1 + sqrt(1 + (8 * fieldCount))) / 2;
}

void printGameBoard()
{
    int rows = rowsFromFieldCount(gameBoardFieldCount);
    printf("Fields: %u, rows: %u\n", gameBoardFieldCount, rows);
    int index = 0;
    for (int row = 0; row < rows; row++) {
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

void generateFollowupStates(State* state, State **followupStates, int *followupStateCount)
{
    // generate
}

void saveResult()
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
                saveResult();
            }
            continue;
        }
        if (state->depth == maxDepth || state->depth == minDepth - 1) continue;
        State* followupStates[6];
        int followupStateCount;
        generateFollowupStates(state, followupStates, &followupStateCount);
        for (int i = 0; i < followupStateCount; i++) {
            pushState(followupStates[i]);
        }
        previousState = state;
    }
    return EXIT_SUCCESS;
}
