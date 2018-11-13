#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void mcmc_start();

void mcmc_stop();

void mcmc_commit(int tid);

void mcmc_abort(int tid);

void mcmc_determine(int tid);

double mcmc_gettime();

void mcmc_timeanalyse(int tid, int start);

void mcmc_throughput(int tid);

void  mcmc_registerTransId(int tid, int transId);
