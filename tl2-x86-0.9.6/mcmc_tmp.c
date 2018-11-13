#include "mcmc.h"
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
    #define NUMTHREADS 16
//#define throughput
    #define TWINDOW 100
#define mcmcdata
//#define modelusage
//#define monitor_ND
//#define allowsingle
//	#define WAITING
  #define breakloop
  #define ALL_MATCH
   #define high
    #define FREQX 2
#define SAME_RETRIES 3
#define MAX_RANKS 6

#ifdef debug
#define Debug printf
#define Debug2 printf
//#define Debug2(x,y) //nothing
#else
#define Debug(x) //nothing
#define Debug2(x,y) //nothing
#endif

#define BUF_SIZE 10000
#define STATE_SIZE 20
// structures of transition matrix
char strbuff[BUF_SIZE] = {'\0'};
int i = 0;
int numthreads = 0;
pthread_mutex_t mclock;
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
char *pmu_tran_states;
char *pmu_cur_states;
void buildstates();
void freestates();
#endif


#define ulong unsigned

ulong transstate = 0;
ulong curstate = 0;

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

//###############################
// mcmc_start: init mclock
//###############################
void mcmc_start(int nthreads)
{
    numthreads = nthreads;
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
    pmu_tran_states = (char*)malloc(sizeof(char)*nthreads);
    pmu_cur_states = (char*)malloc(sizeof(char)*nthreads);
    memset(pmu_tran_states,0,sizeof(char)*numthreads);
    memset(pmu_cur_states,0,sizeof(char)*numthreads);
    buildstates();
#endif
#ifdef ticktock
    if(pthread_mutex_init(&timelock,NULL) != 0){
        printf("\n mutex init failed in mcmc_start\n");
        exit(1);
    }
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
    free(pmu_tran_states);
    free(pmu_cur_states);
    freestates();
#ifdef monitor_ND
    FILE *fp = NULL;
    fp = fopen("mcmc_monitor_ND","a+");
    strbuff[i++] = '\n';
    if(fp == NULL){
        fprintf(stderr,"Error opening mcmc_monitor_ND file");
    }
    fwrite(strbuff,i-1,1,fp);
    fclose(fp);
#endif
#else
#ifdef mcmcdata
    FILE *fp = NULL;
    fp = fopen("mcmc_data","a+");
    strbuff[i++] = '\n';
    if(fp == NULL){
        fprintf(stderr,"Error opening mcmc_data file");
    }
    fwrite(strbuff,i-1,1,fp);
    fclose(fp);
#endif

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
        sprintf(timebuff[tid][ti[tid]],"%f;",(mcmc_gettime()-tstart[tid]));
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
    pmu_tran_states[tid] = 1;
    memcpy(pmu_cur_states,pmu_tran_states,sizeof(char)*numthreads);
    memset(pmu_tran_states,0,sizeof(char)*numthreads);
#ifdef monitor_ND
    pthread_mutex_lock(&mclock);
    unsigned cstate = 0;
    for(int i =0;  i< numthreads; ++i)
    {
        unsigned temp = pmu_cur_cstates[i] << i;
        cstate |= temp;
    }
    curstate = cstate;
    if(curstate <10){
        sprintf(&strbuff[i++],"%d",curstate);
    }else if(curstate >= 10 && curstate < 100){
        sprintf(&strbuff[i],"%d",curstate);
        i +=2;
    }else if(curstate >= 100 && curstate < 1000){
        sprintf(&strbuff[i],"%d",curstate);
        i +=3;
    }else if(curstate >= 1000 && curstate < 10000){
        sprintf(&strbuff[i],"%d",curstate);
        i +=4;
    }else if(curstate >= 10000 && curstate < 100000){
        sprintf(&strbuff[i],"%ld",curstate);
        i +=5;
    }
    //sprintf(&strbuff[i++],"%d",tid);
    strbuff[i++] = ';';
    if(i+5 >= BUF_SIZE){//check if the buff has space for next writes
        FILE *fp = NULL;
        fp = fopen("mcmc_monitor_ND","a+");
        if(fp == NULL){
            fprintf(stderr,"Error opening mcmc_monitor_ND file");
        }
        fwrite(strbuff,i,1,fp);
        memset(strbuff,'\0',BUF_SIZE);
        fclose(fp);
        i = 0;
    }
    pthread_mutex_unlock(&mclock);
#endif
#endif
#ifdef mcmcdata
    pthread_mutex_lock(&mclock);
    unsigned temp = 1 << tid;
    transstate |= temp;


    Debug2("\n Commit transstate = %d", transstate);
    curstate = transstate;
    transstate = 0;

    if(curstate <10){
        sprintf(&strbuff[i++],"%d",curstate);
    }else if(curstate >= 10 && curstate < 100){
        sprintf(&strbuff[i],"%d",curstate);
        i +=2;
    }else if(curstate >= 100 && curstate < 1000){
        sprintf(&strbuff[i],"%d",curstate);
        i +=3;
    }else if(curstate >= 1000 && curstate < 10000){
        sprintf(&strbuff[i],"%d",curstate);
        i +=4;
    }else if(curstate >= 10000 && curstate < 100000){
        sprintf(&strbuff[i],"%ld",curstate);
        i +=5;
    }
    //sprintf(&strbuff[i++],"%d",tid);
    strbuff[i++] = ';';
    if(i+5 >= BUF_SIZE){//check if the buff has space for next writes
        FILE *fp = NULL;
        fp = fopen("mcmc_data","a+");
        if(fp == NULL){
            fprintf(stderr,"Error opening mcmc_data file");
        }
        fwrite(strbuff,i,1,fp);
        memset(strbuff,'\0',BUF_SIZE);
        fclose(fp);
        i = 0;
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
    pmu_tran_states[tid] = 1;
#endif
#ifdef mcmcdata
    pthread_mutex_lock(&mclock);
    unsigned temp = 1 << tid;
    transstate |= temp;
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
        unsigned  state;
        int rank;
        //struct _trans_state *next;
        //making it an array
}trans_state;

typedef struct _state{
        unsigned Cstate;
        trans_state tstates[6];
        unsigned numtstates;
    #ifdef UTHASH_H
    UT_hash_handle hh;
    #endif
}state;

#ifdef UTHASH_H
struct _state *hash_states = NULL;
#endif
state *fata = NULL;
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

state * find_cur_state(unsigned cstate)
{
    state *stRet = NULL;
#ifdef UTHASH_H
    HASH_FIND_INT(hash_states, &cstate, stRet);
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
        unsigned cstate = 0;
        char *temp_a_cstates = (char*)malloc(sizeof(char)*numthreads);
        memcpy(temp_a_cstates,pmu_cur_states,sizeof(char)*numthreads);
        for(int i =0;  i< numthreads; ++i)
        {
            unsigned temp = temp_a_cstates[i] << i;
            cstate |= temp;
        }
        state *currst = find_cur_state(cstate);
        if(currst == NULL)//can be the initial case
            break;
        trans_state *nttstate = &(currst->tstates[0]);
        unsigned tstate = 0;
        tstate = nttstate->state;
        if(tstate & tid)
            break;
        wait--;
    }
#else

        unsigned prevstate = 0;
        int prevsame = 0;
        while(1)
        {
                unsigned cstate = 0;
                pthread_mutex_lock(&mclock);
                cstate = curstate;
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
                        unsigned tstate = 0;
                        tstate = nttstate->state;

                        if(tstate & tid)
                            break;
                }

        }
#endif
#else
    Debug("Start determine\n");
#ifdef breakloop
    unsigned prevstate = 0;
    int prevsame = 0;
#endif
    while(1)
    {
        unsigned cstate = 0;
        char *temp_a_cstates = (char*)malloc(sizeof(char)*numthreads);
        memcpy(temp_a_cstates,pmu_cur_states,sizeof(char)*numthreads);
        int i = 0;
        for(; i < numthreads; ++i)
        {
            unsigned temp = temp_a_cstates[i];
            temp = temp << i;
            cstate |= temp;
        }

//        printf("\n Cstate = %d",cstate);
//        pthread_mutex_lock(&mclock);
//        cstate = curstate;
//        pthread_mutex_unlock(&mclock);

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
        int it_st= 0;
        int found = 0;
        int tryo = 0;
        int highest = 0;
        int cmp = 0;
        while((nttstate != NULL) && (it_st < 6) && (tryo < MAX_RANKS))
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
            unsigned tstate = 0;
            tstate = nttstate->state;
            if(tstate & tid){
                found = 1;
                break;
            }
            ++it_st;
            nttstate = &(currst->tstates[it_st]);

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
    int numlines =getnumlines("state_data");//number of states
        fp = fopen("state_data","r");
        if(fp == NULL){
                fprintf(stderr,"File state_data could not be opened");
                exit(1);
        }
    fata = (state*)malloc(sizeof(state)*numlines);
    memset(fata,0,sizeof(state)*numlines);
    int st = 0;
        while((read = getline(&line, &len, fp)) != -1)
        {
        char *pch= NULL;
        pch = strtok(line,"-");
        if(strlen(pch) >= STATE_SIZE-1)
            continue;
        fata[st].Cstate = atoi(pch);
        #ifdef UTHASH_H
        HASH_ADD_INT(hash_states,Cstate,(&fata[st]));
        #endif
        Debug2("\n%s",pch);
        int i = 0;
        int tran_rank = 0;
        while(pch != NULL && i < 6)
        {
            if(i != 0){
                if(i%2 == 1){
                    fata[st].tstates[tran_rank].state = atoi(pch);
                    Debug2("\tstate= %s",pch);
                }
                else{
                    fata[st].tstates[tran_rank].rank = atoi(pch);
                    ++tran_rank;
                    Debug2("-%s\n",pch);
                }
            }
            int valid_next = 1;
            do{
                pch=strtok(NULL," -:");
                if(pch != NULL && strlen(pch) >= STATE_SIZE-1){
                    valid_next = 0;
                    pch = strtok(NULL," -:");//get the frequency out only look for states
                }else
                    valid_next = 1;
            }while(!valid_next && pch != NULL);
            i++;
        }

        fata[st].numtstates = i+1;
        st++;
    }
    nstates = st;
    fclose(fp);
    Debug("\n states built");fflush(stdout);
}

void printstates()
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
}

void freestates()
{
    Debug("\n In free");fflush(stdout);
#ifdef UTHASH_H
    state *s,*tmp = NULL;
    HASH_ITER(hh,hash_states,s,tmp){
        HASH_DEL(hash_states,s);
    }
#endif

    free(fata);
}
#endif

