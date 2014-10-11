//
//  startup.h
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#ifndef __PEK__startup__
#define __PEK__startup__

#include "bool.h"

void initProcessNums();
void loadInputData(int argc, char *argv[]);
void pushInitialState();
void initialize();

#endif /* defined(__PEK__startup__) */
