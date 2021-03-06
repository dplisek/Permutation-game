//
//  logging.h
//  PEK
//
//  Created by Dominik Plisek on 10/10/14.
//  Copyright (c) 2014 Dominik Plisek a Tomas Marek. All rights reserved.
//

#ifndef __PEK__logging__
#define __PEK__logging__

#ifdef DEBUG
#define INIT_LOG() initLog();
#define LOG(args...) logToFile(args);
#define LOG_GAME() logGameBoardToFile();
#else
#define INIT_LOG()
#define LOG(text, ...)
#define LOG_GAME()
#endif

void initLog();
void logToFile(const char *format, ...);
void logGameBoardToFile();

#endif /* defined(__PEK__logging__) */
