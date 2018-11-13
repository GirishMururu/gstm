#include"perm.h"

#define debug

#ifdef debug
#define DEBUG(x) printf(x); fflush(stdout)
#else
#define DEBUG(x) //nothing
#endif
//Girish
int counter = 0;
int *acquisitionorder = NULL;
double *threadtime = NULL;
pthread_mutex_t plock,reslock;
int numThread = 0;
//pthread_cond_t cond;
//#################################
//Get time
//#################################
double gettime()
{
        struct timespec tv;
        FILE *fp = NULL;
        if(clock_gettime(CLOCK_REALTIME,&tv) != 0){
                 fp = fopen("trialresults.txt","a+");
                 fprintf(fp,"Che timer error\n");
                 fclose(fp);
                 return 0;
        }
        return (((double) tv.tv_sec) +(double)(tv.tv_nsec/1.0e9));
}
//Mururu
long long *transCounter = NULL;
//#################################
//Transaction profile init
//#################################
void trans_profile_init(int nThreads)
{
	numThread = nThreads;
	transCounter = (long long *) malloc(sizeof(long long));
        threadtime = (double*) malloc(sizeof(double)*numThread);
        if(pthread_mutex_init(&reslock,NULL) != 0){
                printf("\n mutex init failed\n");
		trans_profile_destroy();
               	exit(1);
        }
}

//#################################
//Transaction free
//#################################
void trans_profile_destroy()
{
	DEBUG("freeing up Memory");
	free(transCounter);
	free(threadtime);
}
	
//#################################
//Lock based profile init
//#################################
void perm_profile_init(int nThreads)
{
#ifdef PERM_PROFILE
	int i = 0;
	//Girish
        numThread = nThreads;
        acquisitionorder = (int*) malloc(sizeof(int)*numThread);
        threadtime = (double*) malloc(sizeof(double)*numThread);
        if(pthread_mutex_init(&plock,NULL) != 0){
                printf("\n mutex init failed\n");
                free(acquisitionorder);
                free(threadtime);
               	exit(1);
        }
        if(pthread_mutex_init(&reslock,NULL) != 0){
                printf("\n mutex init failed\n");
                free(acquisitionorder);
                free(threadtime);
               	exit(1);
        }
	
	for(i =0; i < numThread; i++)
	{
		threadtime[i] = gettime();
	}
        //Mururu
#endif
}

//#################################
//lock perm profile 
//#################################
void perm_profile_destroy()
{
#ifdef PERM_PROFILE
	free(acquisitionorder);
	free(threadtime);
#endif
}

//#################################
//Trans based counter inc
//#################################
int  get_count(int tid)
{
	int iret = 0;
        //printf("\n count acquired by %d", tid);
	//printf("Get Count %d\n",tid);
	if(threadtime)
	threadtime[tid] = gettime();
	if(transCounter)
	 iret = (*transCounter)++;
	else
	iret = -1;

	return iret;
}

//#################################
//Trans counter log
//#################################
void log_count(int tid,int count)
{

        //printf("\n Log by %d", tid);
        FILE *fp;
        fp = fopen("trialresults.txt","a+");
        fprintf(fp,"%d,",tid);
	//printf("Log Count %d\n",tid);
	if(threadtime)
        fprintf(fp,"%f,",threadtime[tid]);
        fprintf(fp,"%d\n",count);
        fclose(fp);
}
//#################################
//Lock based log
//#################################
void perm_log(int tid)
{
#ifdef PERM_PROFILE
	//Girish
        threadtime[tid] = gettime();// - threadtime[tid];
	pthread_mutex_lock(&plock);
  //      printf("\n pLock acquired by %d", tid);
        acquisitionorder[tid] = counter++;
        pthread_mutex_unlock(&plock);
//	printf("\npLock released by %d", tid);
	
/*        FILE *fp;
//	pthread_mutex_lock(&reslock);
        printf("\n resLock acquired by %d", tid);
        fp = fopen("trialresults.txt","a+");
        fprintf(fp,"%d,",tid);
        fprintf(fp,"%f,",threadtime[tid]);
        fprintf(fp,"%d\n",acquisitionorder[tid]);
        fclose(fp);
  //      pthread_mutex_unlock(&reslock);
	printf("\nreslock released by %d", tid);
	
*/	
        acquisitionorder[tid] = 0;
//	threadtime[tid] = gettime();
#endif
}
