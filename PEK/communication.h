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
#include "color.h"
#ifdef STAR
#include "mpi.h"
#else
#include "/usr/local/include/mpi.h"
#endif

void sendStatesWithCommonParentToProcess(State **states, int stateCount, int process);
void requestWork();
void handOutInitialDataToAllProcesses();
void broadcastFinish();
void sendTokenTo(int dest, COLOR color);
void handleWorkRequestFrom(int source);
void handleWorkResponseFrom(int source);
void handleNoWorkFrom(int source);
void handleTokenFrom(int source);
void handleFinishFrom(int source);
void handleInitialDataFrom(int source);
void collectResults();

#endif /* defined(__PEK__communication__) */
