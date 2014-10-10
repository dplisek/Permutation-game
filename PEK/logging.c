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

char logFileName[256];
extern int processNum;

void initLog()
{
    sprintf(logFileName, LOG_FILENAME_FORMAT, processNum);
    remove(logFileName);
}

void logToFile(const char *format, ...)
{
    va_list arguments;
    FILE *logFile = fopen(logFileName, "a");
    va_start(arguments, format);
    vfprintf(logFile, format, arguments);
    va_end(arguments);
    fclose(logFile);
}
