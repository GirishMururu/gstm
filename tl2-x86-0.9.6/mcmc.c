#include "mcmc.h"
#include <math.h>
#include "uthash.h"
//############################################################
//
// MCMC: Markov Chain Monte Carlo
//
//	all mcmc functions needed to develop the markov model
//#############################################################

//#define debug

//#define abortvar
#ifdef abortvar

 int naborts;
 int ncommits;
 double tlast;
 #define ATHREAD 3
 #define AWINDOW 100

#endif
//#define ticktock
    #define NUMTHREADS 8
//#define throughput
    #define TWINDOW 100
//#define mcmcdata
//#define modelusage
//#define monitor_ND
//#define allowsingle
//	#define WAITING
  #define breakloop
  #define ALL_MATCH
   #define high
   // #define FREQX 2
#define SAME_RETRIES 2
//#define MAX_RANKS 6

#ifdef debug
#define Debug printf
#define Debug2 printf
//#define Debug2(x,y) //nothing
#else
#define Debug(x) //nothing
#define Debug2(x,y) //nothing
#endif


#ifdef monitor_ND
#define BUF_SIZE 100000
#define STATE_SIZE 24
int numaborts[NUMTHREADS] = {0};
FILE** abortFp =NULL;
char abortBuff[NUMTHREADS][BUF_SIZE][12] = {{{'\0'}}};
int abortBufIndex[NUMTHREADS] = {0};
#else
#define BUF_SIZE 10000
#endif
// structures of transition matrix
char strbuff[BUF_SIZE] = {'\0'};
//char Dummy[100] = {0};
int i = 0, ist = 0;
int numthreads = 0;
//char Dummy1[100] = {0};
pthread_mutex_t mclock;
//char Dummy2[100] = {0};
#ifdef ticktock
pthread_mutex_t timelock;
char timebuff[NUMTHREADS][BUF_SIZE][20] = {{{'\0'}}};
double *tstart =NULL;
int *cycle =NULL;
int ti[NUMTHREADS] = {0};
#endif
#ifdef throughput
int *tput = NULL;
double *tlasttime =NULL;
#endif
#ifdef modelusage
 //char *pmu_tran_states; 
 //char *pmu_cur_states;
 void buildstates();
 void freestates();
#endif


typedef  unsigned long ulong;

//ulong transstate = 0;
//ulong curstate = 0;

char  *captureTransId =  NULL;
//struct with bits for transaction of each thread
typedef union
{
    struct
    {
      unsigned char commitThread :4;
      
      unsigned char bit1 : 3;
      unsigned char bit2 : 3;
      unsigned char bit3 : 3;
      unsigned char bit4 : 3;
      unsigned char bit5 : 3;
      unsigned char bit6 : 3;
      unsigned char bit7 : 3;
      unsigned char bit8 : 3;
      unsigned char bit9 : 3;
      unsigned char bit10 : 3;
      unsigned char bit11 : 3;
      unsigned char bit12 : 3;
      unsigned char bit13 : 3;
      unsigned char bit14 : 3;
      unsigned char bit15 : 3;
      unsigned char bit16 : 3;
      unsigned int unused :12;//this union is of 64 bits
      
    }thread;
  long long state;
}tmState;

tmState s_transState = {0};
long long u_cur_state = 0;

/*#ifdef monitor_ND
char strcurstate[STATE_SIZE] ={'\0'},strtransstate[STATE_SIZE] = {'\0'};
#endif*/

//#################################
//Get time
//#################################
double mcmc_gettime()
{
    struct timespec tv = {0};
    FILE *fp = NULL;
    if(clock_gettime(CLOCK_REALTIME,&tv) != 0){
        fp = fopen("trialresults.txt","a+");
        fprintf(fp,"Che timer error\n");
        fclose(fp);
        return 0;
    }
    return (((double) tv.tv_sec) +(double)(tv.tv_nsec/1.0e9));
}
/***************************************
mcmc_registerTransId(thread, transactionId)
****************************************/
inline void mcmc_registerTransId(int threadId, int transId)
{
  captureTransId[threadId] = transId;
}

//###############################
// mcmc_start: init mclock
//###############################
void mcmc_start(int nthreads)
{
    numthreads = nthreads;
    captureTransId = (char*)malloc(sizeof(char)*numthreads);
    memset(captureTransId,-1,sizeof(char)*numthreads);
#ifdef abortvar
    naborts =0;
    ncommits = 0;
    tlast = mcmc_gettime();
    return;
#endif
    if(pthread_mutex_init(&mclock,NULL) != 0){
        printf("\n mutex init failed in mcmc_start\n");
        exit(1);
    }
    Debug("In start\n"); fflush(stdout);
#ifdef modelusage
    Debug("\n Model Usage");
    buildstates();
#endif
#ifdef ticktock
    tstart = (double*)malloc(sizeof(double)*nthreads);
    cycle = (int*)malloc(sizeof(int)*nthreads);
    memset(tstart,0,sizeof(double)*nthreads);
    memset(cycle,0,sizeof(int)*nthreads);
#endif
#ifdef throughput
    tput = (int*)malloc(sizeof(int)*nthreads);
    tlasttime= (double*)malloc(sizeof(double)*nthreads);
    memset(tlasttime,0,sizeof(double)*nthreads);
    memset(tput,0,sizeof(int)*nthreads);
    int index = 0;
    for(; index< nthreads; index++)
        tlasttime[index] = mcmc_gettime();
#endif

#ifdef monitor_ND
   abortFp = (FILE**)malloc(sizeof(FILE*)*NUMTHREADS);
    for(int i =0; i < NUMTHREADS; ++i)
    {
	char filename[20];
	sprintf(filename,"abortsThread%d",i);
	FILE *fp = fopen(filename,"a+");
	if(fp==NULL){
	 fprintf(stderr,"Error opening %s",filename);
	 break;
	}
	abortFp[i]=fp;
    }
#endif

}
//#####################################
// mcmc_stop: dump the buffer
//#####################################
void mcmc_stop()
{
#ifdef abortvar
    return;
#endif
    Debug("In stop\n"); fflush(stdout);
#ifdef modelusage
    freestates();
#else
#ifdef mcmcdata
    FILE *fp = NULL;
    fp = fopen("mcmc_data","a+");
    strbuff[i++] = '\n';
    if(fp == NULL){
        fprintf(stderr,"Error opening mcmc_data file");
    }
    fwrite(strbuff,1,i-1,fp);
    fclose(fp);
#endif
#endif

#ifdef monitor_ND
    FILE *fp = NULL;
    fp = fopen("mcmc_monitor_ND","a+");
    strbuff[i++] = '\n';
    if(fp == NULL){
        fprintf(stderr,"Error opening mcmc_monitor_ND file");
    }
    fwrite(strbuff,1,i-1,fp);
    fclose(fp);
  for(int ij =0; ij<NUMTHREADS;++ij)
  {
    int index=abortBufIndex[ij];
    for(int ki=0, ke=abortBufIndex[ij];ki<=ke;++ki)
	fprintf(abortFp[ij],"%s",abortBuff[ij][ki]);
    fclose(abortFp[ij]);
  }
  free(abortFp);
/*fp = fopen("thread_aborts","a+");
    if(fp == NULL){
	fprintf(stderr,"Error opening thread_aborts file\n");
    }
    int t = 0;
    for( ; t < numthreads; ++t)
    {
	fprintf(fp,"%d,",numaborts[t]);
    }
    fprintf(fp,";");
    fclose(fp);
*/
#endif

#ifdef ticktock
    int iti = 0;
    for(; iti < NUMTHREADS; iti++)
    {
        FILE *fp =NULL;
        char tfile[20] ={'\0'};
        sprintf(tfile, "timeAna%d",iti);
        fp = fopen(tfile,"a+");
        if(fp == NULL){
            fprintf(stderr, "could not open timeanalysis");
            free(tstart);
            free(cycle);
            return;
        }
        int k = 0;
        for(k = 0; k <= ti[iti]; k++)
            fprintf(fp,"%s",timebuff[iti][k]);
        //memset(timebuff[iti],'\0',sizeof(timebuff));
        fclose(fp);
    }
    free(tstart);
    free(cycle);

#endif
#ifdef throughput
    free(tput);
    free(tlasttime);
#endif
}

//######################################################
// mcmc_throughput : anlaysis of throughtput per thread
//######################################################
void mcmc_throughput(int tid)
{
#ifdef throughput
    tput[tid]++;
    if((mcmc_gettime() - tlasttime[tid])*1000 > TWINDOW)
    {
        char filename[10] = {'\0'};
        sprintf(filename,"tput_%d",tid);
        FILE *fp = NULL;
        fp= fopen(filename,"a+");
        if(fp == NULL)
        {
            fprintf(stderr,"could not open file %s",filename);
            return ;
        }
        fprintf(fp,"%d,",tput[tid]);
        fclose(fp);
        tput[tid] = 0;
        tlasttime[tid] = mcmc_gettime();
    }
#endif
}

//################################################
// mcmc_timeanalyse : anlaysis of time per thread
//################################################
void mcmc_timeanalyse(int tid, int start)
{
#ifdef ticktock
    if(start){
        if(!cycle[tid]){
            tstart[tid] = mcmc_gettime();
            cycle[tid] = 1;
        }
    }else{//stop
        //	pthread_mutex_lock(&timelock);
      sprintf(timebuff[tid][ti[tid]],"%d@%f;",captureTransId[tid],(mcmc_gettime()-tstart[tid]));
        ti[tid]++;
        if(ti[tid] >= BUF_SIZE){
            FILE *fp =NULL;
            char tfile[20] ={'\0'};
            sprintf(tfile, "timeAna%d",tid);
            fp = fopen(tfile,"a+");
            if(fp == NULL){
                fprintf(stderr, "could not open timeanalysis");
                return;
            }
            int k = 0;
            for(k = 0; k <= ti[tid]; k++)
                fprintf(fp,"%s",timebuff[tid][k]);
            memset(timebuff[tid],'\0',sizeof(timebuff[tid]));
            fclose(fp);
            ti[tid] = 0;

        }
        //	pthread_mutex_unlock(&timelock);
        cycle[tid] = 0;
    }
#endif
}/*********************************************
inline void get_transState(int tid)
 ********************************************/
static inline unsigned char  get_transState(tmState *st_transState,int tid)
{
switch(tid){
    case 0:
      return st_transState->thread.bit1 ;
      break;
    case 1:
      return st_transState->thread.bit2 ;
      break;
    case 2:
      return st_transState->thread.bit3 ;
      break;
    case 3:
      return st_transState->thread.bit4 ;
      break;
    case 4:
      return st_transState->thread.bit5 ;
      break;
    case 5:
      return st_transState->thread.bit6 ;
      break;
    case 6:
      return st_transState->thread.bit7 ;
      break;
    case 7:
      return st_transState->thread.bit8 ;
      break;
    case 8:
      return st_transState->thread.bit9 ;
      break;
    case 9:
      return st_transState->thread.bit10 ;
      break;
    case 10:
      return st_transState->thread.bit11 ;
      break;
    case 11:
      return st_transState->thread.bit12 ;
      break;
    case 12:
      return st_transState->thread.bit13 ;
      break;
    case 13:
      return st_transState->thread.bit14 ;
      break;
    case 14:
      return st_transState->thread.bit15 ;
      break;
    case 15:
      return st_transState->thread.bit16 ;
      break;
    default:
      fprintf(stderr," This thread Id must not be run here\n");
      break;
    }
 return 0;
}
/*********************************************
inline void set_transState(int tid)
 ********************************************/
static inline void set_transState(tmState *st_transState,int tid)
{
switch(tid){
    case 0:
      st_transState->thread.bit1 = captureTransId[tid];
      break;
    case 1:
      st_transState->thread.bit2 = captureTransId[tid];
      break;
    case 2:
      st_transState->thread.bit3 = captureTransId[tid];
      break;
    case 3:
      st_transState->thread.bit4 = captureTransId[tid];
      break;
    case 4:
      st_transState->thread.bit5 = captureTransId[tid];
      break;
    case 5:
      st_transState->thread.bit6 = captureTransId[tid];
      break;
    case 6:
      st_transState->thread.bit7 = captureTransId[tid];
      break;
    case 7:
      st_transState->thread.bit8 = captureTransId[tid];
      break;
    case 8:
      st_transState->thread.bit9 = captureTransId[tid];
      break;
    case 9:
      st_transState->thread.bit10 = captureTransId[tid];
      break;
    case 10:
      st_transState->thread.bit11 = captureTransId[tid];
      break;
    case 11:
      st_transState->thread.bit12 = captureTransId[tid];
      break;
    case 12:
      st_transState->thread.bit13 = captureTransId[tid];
      break;
    case 13:
      st_transState->thread.bit14 = captureTransId[tid];
      break;
    case 14:
      st_transState->thread.bit15 = captureTransId[tid];
      break;
    case 15:
      st_transState->thread.bit16 = captureTransId[tid];
      break;
    default:
      fprintf(stderr," This thread Id must not be run here\n");
      break;
    }
 return;
}
//##############################################
// mcmc_commit : append commit order to strbuff
//##############################################
void mcmc_commit(int tid)
{
#ifdef abortvar
    if(tid == ATHREAD)
    {
        ncommits++;
    }
    if((mcmc_gettime() -tlast)*1000 >= AWINDOW)
    {
        FILE *fp = NULL;
        fp = fopen("Apcwindow","a+");
        if(fp)
        {
            int put = ncommits > 0 ? (naborts/ncommits): 0;
            fprintf(fp,"%d,",put);
            fclose(fp);
            naborts = 0;
            ncommits = 0;
            tlast = mcmc_gettime();
        }
        else
            fprintf(stderr,"cannot open the file");
    }

    return;
#endif


#ifdef modelusage
    pthread_mutex_lock(&mclock);
	    set_transState(&s_transState,tid);
	    s_transState.thread.commitThread = tid;
	    u_cur_state = s_transState.state;
	    s_transState.state = 0;
    pthread_mutex_unlock(&mclock);
#endif
#ifdef monitor_ND
    
    pthread_mutex_lock(&mclock);
    //sprintf(&transstate[ist++],"%d",tid);
    //strtransstate[ist++] = (char)(((int)'0')+tid);
    //Debug2("\n Commit transstate = %s", strtransstate);
    //memset(strcurstate,'\0',sizeof(strcurstate));
//	if(strlen(transstate) < STATE_SIZE - 1)
    //strncpy(strcurstate,strtransstate,strlen(strtransstate));
    //memset(strtransstate,'\0',sizeof(strtransstate));
    //ist = 0;
    if(tid < 10){
    	sprintf(&strbuff[i],"%d@%d",captureTransId[tid],tid);
	i +=3;
    }else{
    	sprintf(&strbuff[i],"%d@%d",captureTransId[tid],tid);
	i += 4;
    }
    strbuff[i++] = ';';
    if(i >= BUF_SIZE -5){//check if the buff has space for next writes
        FILE *fp = NULL;
        fp = fopen("mcmc_monitor_ND","a+");
        if(fp == NULL){
            fprintf(stderr,"Error opening mcmc_monitor_ND file");
        }
        fwrite(strbuff,1,i,fp);
        memset(strbuff,'\0',BUF_SIZE);
        fclose(fp);
        i = 0;
    }

    pthread_mutex_unlock(&mclock);

    {
      /*int incBuf = (int)(log10(numaborts[tid])) + 2;//2 for transId and @
      if(abortBufIndex[tid]+incBuf >= BUF_SIZE-1){//check if the buff has space for next writes
	    fwrite(abortBuff[tid],1,abortBufIndex[tid],abortFp[tid]);
	    memset(abortBuff[tid],'\0',BUF_SIZE);
	    abortBufIndex[tid]= 0;
	}*/
	
      int index = abortBufIndex[tid];
	if(index >= BUF_SIZE-1){
	    for(int ki=0, ke=index; ki <=ke;++ki)
		fprintf(abortFp[tid],"%s",abortBuff[tid][ki]);
	    fprintf(abortFp[tid],"\n");
            memset(abortBuff[tid],'\0',sizeof(abortBuff[tid]));
	  index=0;
	}
      sprintf((abortBuff[tid][index]),"%d@%d;",captureTransId[tid],numaborts[tid]);
      numaborts[tid]=0;
      index++;
      abortBufIndex[tid] = index;
    }
    
#endif

#ifdef mcmcdata

    pthread_mutex_lock(&mclock);
    {
        set_transState(&s_transState,tid);
	s_transState.thread.commitThread = tid;
	long long  llcurstate = s_transState.state;
	s_transState.state = 0;
	Debug2("\n Commit transstate = %lld", llcurstate);
	int incBuf = (int)(log10(llcurstate)) + 1;
	if(i+incBuf >= BUF_SIZE-1){//check if the buff has space for next writes
	    FILE *fp = NULL;
	    fp = fopen("mcmc_data","a+");
	    if(fp == NULL){
		fprintf(stderr,"Error opening mcmc_data file");
	    }
	    fwrite(strbuff,1,i,fp);
	    memset(strbuff,'\0',BUF_SIZE);
	    fclose(fp);
	    i = 0;
	}
	sprintf(&strbuff[i],"%lld",llcurstate);
	i += incBuf;
	strbuff[i++] = ';';
    }
    pthread_mutex_unlock(&mclock);
#endif

}

//#############################################
//mcmc_abort: append the concurrent threads
//#############################################
void mcmc_abort(int tid)
{

#ifdef abortvar
    if(tid == ATHREAD)
        naborts++;
    return;
#endif
#ifdef modelusage
    pthread_mutex_lock(&mclock);
    set_transState(&s_transState,tid);
    pthread_mutex_unlock(&mclock);
#endif
#ifdef mcmcdata
    pthread_mutex_lock(&mclock);
     set_transState(&s_transState,tid);
    pthread_mutex_unlock(&mclock);
#endif

#ifdef monitor_ND
    numaborts[tid]++;
    pthread_mutex_lock(&mclock);
    /*sprintf(&transstate[ist],"%d,",tid);
    strtransstate[ist++] = (char)(((int)'0')+tid);
    strtransstate[ist++] = ',';
    if(ist > STATE_SIZE-3){
        memset(strtransstate,'\0',sizeof(strtransstate));
        ist = 0;
    }
    Debug2("\nAbort transstate = %s", strtransstate);
    */
    if(tid < 10){
      sprintf(&strbuff[i],"%d@%d",captureTransId[tid],tid);
	i +=3;
    }else{
    	sprintf(&strbuff[i],"%d@%d",captureTransId[tid],tid);
	i += 4;
    }
    strbuff[i++] = ',';
    if(i >= BUF_SIZE -5){//check if it fills for next two writes
        FILE *fp = NULL;
        fp = fopen("mcmc_monitor_ND","a+");
        if(fp == NULL){
            fprintf(stderr,"Error opening mcmc_monitor_ND file");
        }
        fwrite(strbuff,1,i,fp);
        memset(strbuff,'\0',BUF_SIZE);
        fclose(fp);
        i = 0;
    }
    pthread_mutex_unlock(&mclock);
#endif

}

#ifndef modelusage
void mcmc_determine(int tid){}//nothing
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Usage of model generated
//
/////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef modelusage
typedef struct _trans_state{
        long long  state;
        int rank;
        //struct _trans_state *next;
        //making it an array
}trans_state;

typedef struct _state{
        long long Cstate;
        trans_state tstates[MAX_RANKS];
        unsigned numtstates;
    #ifdef UTHASH_H
    UT_hash_handle hh;
    #endif
}state;

#ifdef UTHASH_H
struct _state *hash_states = NULL;
#endif
//state *fata = NULL;
int nstates = 0;


int getnumlines(char *filename)
{
    int ch;
    FILE *fp = fopen(filename,"r");
    if(fp ==NULL){
        fprintf(stderr,"Could not open file to get number of lines");
        exit(1);
    }
    int lcount = 0;
    while((ch = fgetc(fp)) != EOF)
    {
        if(ch == '\n')
            lcount++;
    }
    fclose(fp);
    return lcount;
}

state* find_cur_state(long long cstate)
{
    state *stRet = NULL;
#ifdef UTHASH_H
    //HASH_FIND_INT(hash_states, &cstate, stRet);
    HASH_FIND(hh,hash_states, &cstate,sizeof(cstate),stRet);
#else
    int in = 0;
    while(in < nstates)
    {
        if(fata[in].Cstate == cstate){
            stRet = &fata[in];
            break;
        }
        in++;
    }
#endif
    return stRet;
}

//###################################################
//mcmc_determine: make threads less non-determinstic
//###################################################
void mcmc_determine(int tid)
{
#ifdef allowsingle
#ifdef WAITING
    int wait = 30;
    while(wait)
    {
        long long cstate = u_cur_state;
        state *currst = find_cur_state(cstate);
        if(currst == NULL)//can be the initial case
            break;
        trans_state *nttstate = &(currst->tstates[0]);
        long long  tstate = 0;
        tstate = nttstate->state;
        if(tstate & tid)
            break;
        wait--;
    }
#else

        long long prevstate = 0;
        int prevsame = 0;
        while(1)
        {
                long long cstate = 0;
                pthread_mutex_lock(&mclock);
                cstate = u_curstate;
                pthread_mutex_unlock(&mclock);
        #ifdef breakloop
                if(cstate == prevstate){
                        prevsame++;
                        if(prevsame > SAME_RETRIES)
                          break;
                }else{
                        prevstate = cstate;
                        prevsame = 0;
                }
        #endif
                state *currst = find_cur_state(cstate);
                if(currst == NULL)//can be the initial case
                        break;
                trans_state *nttstate = &(currst->tstates[0]);
                int found = 0;
                {
                        long long  tstate = 0;
                        tstate = nttstate->state;
			tmState tmpState = {0};
			tmpState.state = tstate;
			unsigned char transId = captureTransId[tid];
                        if((transId ^ get_transState(&tmpState,tid)) == 0)
                            break;
                }

        }
#endif
#else
    Debug("Start determine\n");
#ifdef breakloop
    long long  prevstate = 0;
    int prevsame = 0;
#endif
    while(1)
    {
//        printf("\n Cstate = %d",cstate);
//        pthread_mutex_lock(&mclock);
//        cstate = curstate;
//        pthread_mutex_unlock(&mclock);

        long long cstate = u_cur_state;
        #ifdef breakloop
        if(cstate == prevstate){
            prevsame++;
            if(prevsame > SAME_RETRIES)
              break;
            continue;
        }else{
             prevstate = cstate;
             prevsame = 0;
        }
        #endif
        state *currst = find_cur_state(cstate);
        if(currst == NULL)//can be the initial case
            break;
        trans_state *nttstate = &(currst->tstates[0]);
        int found = 0;
        int tryo = 0;
        int highest = 0;
        int cmp = 0;
        while((nttstate != NULL) &&(tryo < currst->numtstates) && (tryo < MAX_RANKS))
        {
            #ifdef high
            if (tryo != 0){
                if(nttstate->rank < cmp)
                  break;
            }
            else{
                highest = nttstate->rank;
                cmp = highest/FREQX;
            }
            ++tryo;
            #endif
            long long tstate = 0;
            tstate = nttstate->state;
	    tmState tmpState = {0};
	    tmpState.state = tstate;
	    unsigned char transId = captureTransId[tid];
            if((transId ^ get_transState(&tmpState,tid)) == 0){
                found = 1;
                break;
            }
            nttstate = &(currst->tstates[tryo]);
        }

        if(found)
          break;
    }
    Debug("stop determine");

#endif
}


void buildstates()
{
        FILE *fp;
        char *line =NULL;
        size_t len = 0;
        ssize_t read;

    Debug("\n building automata\n");
    // int numlines =getnumlines("state_data");//number of states
        fp = fopen("state_data","r");
        if(fp == NULL){
                fprintf(stderr,"File state_data could not be opened");
                exit(1);
        }
    //fata = (state*)malloc(sizeof(state)*numlines);
    //memset(fata,0,sizeof(state)*numlines);
    int st = 0;
    while((read = getline(&line, &len, fp)) != -1)
    {
        state *fata = (state*)malloc(sizeof(state));
        char *pch= NULL;
        pch = strtok(line,"-");

        fata->Cstate = atoll(pch);
        #ifdef UTHASH_H
        //HASH_ADD_INT(hash_states,Cstate,(&fata[st]));
        HASH_ADD(hh,hash_states,Cstate,sizeof(long long),fata);
        #endif
        Debug2("\n%s",pch);
        int i = 0;
        int tran_rank = 0;
        while(pch != NULL && i < MAX_RANKS)
        {
            if(i != 0){
                if(i%2 == 1){
                    fata->tstates[tran_rank].state = atoll(pch);
                    Debug2("\tstate= %s",pch);
                }
                else{
                    fata->tstates[tran_rank].rank = atoll(pch);
                    ++tran_rank;
                    Debug2("-%s\n",pch);
                }
            }
            pch=strtok(NULL," -:");
            i++;
        }

        fata->numtstates = tran_rank;
        st++;
    }
    nstates = st;
    fclose(fp);
    Debug("\n states built");fflush(stdout);
}

	/*void printstates()
{
    int i =0;
    while(i <nstates)
    {
        printf("\n%x",fata[i].Cstate);
        int j = 0;
        while(j < fata[i].numtstates)
        {
            printf("\t %x:%d",fata[i].tstates[j].state,fata[i].tstates[j].rank);
            ++j;
        }
        ++i;
    }
    }*/

void freestates()
{
    Debug("\n In free");fflush(stdout);
#ifdef UTHASH_H
    state *s,*tmp = NULL;
    HASH_ITER(hh,hash_states,s,tmp){
        HASH_DEL(hash_states,s);
	free(s);
    }
#endif
}
#endif
