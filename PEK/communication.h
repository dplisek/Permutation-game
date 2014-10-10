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

BOOL sendStatesWithCommonParentToProcess(State **states, int stateCount, int process);
BOOL receiveWorkFromSource(int source);
void requestWorkFrom(int dest);
void evaluateMessageWithTagFromSource(int tag, int source);
void handOutStatesToAllProcesses();

#endif /* defined(__PEK__communication__) */
