//
//  logging.c
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#include "logging.h"
#include <stdio.h>
#include <stdarg.h>
#include "config.h"
#include "communication.h"
#include "output.h"

char logFileName[256];
extern int processNum;
extern double execTime;

void initLog()
{
    sprintf(logFileName, LOG_FILENAME_FORMAT, processNum);
    remove(logFileName);
}

void logToFile(const char *format, ...)
{
    va_list arguments;
    char logText[1024];
    FILE *logFile = fopen(logFileName, "a");
    va_start(arguments, format);
    vsprintf(logText, format, arguments);
    fprintf(logFile, "[%.4f]: %s", MPI_Wtime() - execTime, logText);
    va_end(arguments);
    fclose(logFile);
}

void logGameBoardToFile()
{
    FILE *logFile = fopen(logFileName, "a");
    printGameBoardToStream(logFile);
    fclose(logFile);
}