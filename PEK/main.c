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

//////////////////////////////////////

#pragma mark - Structs

typedef struct {
    unsigned row, col;
} Coords;

typedef struct {
    Coords blankMovedTo;
} Step;

typedef struct {
    Coords blankCoords;
    unsigned depth;
} State;

typedef struct {
    Step *steps;
    unsigned size;
} StepStack;

typedef struct {
    State *states;
    unsigned size;
} StateStack;

//////////////////////////////////////

#pragma mark - Global variables

#pragma mark Input

unsigned *gameBoard;
unsigned gameBoardRowCount;
unsigned gameBoardFieldCount;
unsigned maxDepth;

#pragma mark Output

StepStack *result;
unsigned minDepth = UINT_MAX;

#pragma mark Runtime

StepStack stepStack;
StateStack stateStack;

//////////////////////////////////////

#pragma mark - Support functions

unsigned fieldCountFromRows(unsigned rows)
{
    return rows * (rows + 1) / 2;
}

unsigned rowsFromFieldCount(unsigned fieldCount)
{
    // solve quadratic function, ignore negative value
    return (-1 + sqrt(1 + (8 * fieldCount))) / 2;
}

void printGameBoard()
{
    printf("Fields: %u, rows: %u\n", gameBoardFieldCount, gameBoardRowCount);
    int index = 0;
    for (int row = 0; row < gameBoardRowCount; row++) {
        for (int col = 0; col <= row; col++) {
            printf("%u\t", gameBoard[index]);
            index++;
        }
        printf("\n");
    }
}

State *findInitialState()
{
    int index = 0;
    for (unsigned row = 0; row < gameBoardRowCount; row++) {
        for (unsigned col = 0; col <= row; col++) {
            unsigned value = gameBoard[index];
            if (value == 0) {
                State *state = (State *)malloc(sizeof(State));
                state->depth = 0;
                Coords coords;
                coords.row = row;
                coords.col = col;
                state->blankCoords = coords;
                return state;
            }
            index++;
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
    unsigned gameBoardFieldCountAllocated = GAME_BOARD_INITIAL_ALLOC;
    gameBoard = (unsigned *)malloc(sizeof(unsigned) * gameBoardFieldCountAllocated);
    unsigned value;
    fscanf(file, "%u", &value);
    while (!feof(file)) {
        gameBoardFieldCount++;
        if (gameBoardFieldCount > gameBoardFieldCountAllocated) {
            gameBoardFieldCountAllocated *= gameBoardFieldCountAllocated;
            gameBoard = realloc(gameBoard, sizeof(unsigned) * gameBoardFieldCountAllocated);
        }
        gameBoard[gameBoardFieldCount - 1] = value;
        fscanf(file, "%u", &value);
    }
    fclose(file);
    gameBoardRowCount = rowsFromFieldCount(gameBoardFieldCount);
    printGameBoard();
    return YES;
}

void pushState(State *state)
{
    stateStack.states[stateStack.size];
}

//////////////////////////////////////

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
    return EXIT_SUCCESS;
}
