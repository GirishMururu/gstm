#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#define PERM_PROFILE
#ifndef PERM_PROFILE
#include "stm.h"
#endif


//#################################
//Lock based Profiling
//#################################
//memory allocation
void perm_profile_init(int nThreads);

//free memory
void perm_profile_destroy();

//log to trialresults.txt
void perm_log(int tid);
//#################################
//Transaction based Profiling
//#################################
//memory allocation
void trans_profile_init(int nThreads);

//free memory
void trans_profile_destroy();

int get_count(int tid);

void log_count(int tid, int count);

