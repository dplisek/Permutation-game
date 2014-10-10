//
//  communication.h
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#ifndef __PEK__communication__
#define __PEK__communication__

#include "statestack.h"
#include "bool.h"
#ifdef STAR
#include "mpi.h"
#else
#include "/usr/local/include/mpi.h"
#endif

#define TAG_INITIAL_WORK 100

BOOL sendStatesWithCommonParentToProcess(State **states, int stateCount, int process);
BOOL receiveWork();

#endif /* defined(__PEK__communication__) */
