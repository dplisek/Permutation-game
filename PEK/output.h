//
//  output.h
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#ifndef __PEK__output__
#define __PEK__output__

#include <stdio.h>
#include "statestack.h"
#include "bool.h"

void printGameBoardToStream(FILE *stream);
BOOL isFinal(State *state);
void saveResultIfBetter(State *state);

#endif /* defined(__PEK__output__) */
