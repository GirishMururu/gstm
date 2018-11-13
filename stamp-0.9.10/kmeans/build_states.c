#include<stdio.h>
#include<string.h>
#include<stdlib.h>

//#define debug

#ifdef debug
#define Debug2 printf
#else
#define Debug2(x,y) //nothing 
#endif

typedef struct _trans_state{
        char  state_name[20];
        int rank;
        struct _trans_state *next;
}trans_state;

typedef struct _state{
        char state_name[20];
        trans_state *roottstate;//Linked list of next possible states
	trans_state *lasttstate;
}state;

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
	
	
void build_states()
{
        FILE *fp;
        char *line =NULL;
        size_t len = 0;
        ssize_t read;
        
	int numlines =getnumlines("state_data");//number of states
        fp = fopen("state_data","r");
        if(fp == NULL){
                fprintf(stderr,"File state_data could not be opened");
                exit(1);
        }
	fata = (state*)malloc(sizeof(state)*numlines);
	int st = 0;	
        while((read = getline(&line, &len, fp)) != -1)
        {
		char *pch;
		pch = strtok(line,"-");
		strcpy(fata[st].state_name,pch);
		fata[st].roottstate = fata[st].lasttstate = NULL;
		Debug2("\n%s",pch);
		int i = 0;
		int rank = 0;
		while(pch != NULL)
		{
			if(i != 0){
				if(i%2 == 1){
					trans_state *tmp = (trans_state*)malloc(sizeof(trans_state));
					strcpy(tmp->state_name,pch);	
					if(fata[st].roottstate == NULL){
						fata[st].roottstate = tmp;
						fata[st].lasttstate= tmp;
					}else{
						fata[st].lasttstate->next = tmp;
						fata[st].lasttstate = tmp;
						
					}
					Debug2("\tstate= %s",pch);
				}
				else{
					fata[st].lasttstate->rank = rank++;
					fata[st].lasttstate->next = NULL;					
				 	Debug2("-%s\n",pch);
				}
			}
			pch=strtok(NULL," -:");
			i++;
		}
		st++;
	}
	nstates = st;
	fclose(fp);
}
void printstates()
{
	int i =0;
	while(i <nstates)
	{
		printf("\n%s",fata[i].state_name);
		trans_state *tmp = fata[i].roottstate;
		while(tmp != NULL)
		{
			printf("\t %s:%d",tmp->state_name,tmp->rank);
			tmp = tmp->next;
		}
		i++;
	}
}
	
void freestates()
{
	int i = 0;
	while(i<nstates)
	{
		trans_state *tmp = fata[i].roottstate;
		while(tmp != NULL)
		{	
			trans_state *prev = tmp;
			tmp = tmp->next;
			Debug2("%d,",i);
			free(prev);
		}
		i++;
	}
	free(fata);
}

int main()
{

	build_states();
	printstates();
	freestates();
	return 0;
}
