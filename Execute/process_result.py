#!/usr/bin/python

import  sys, operator

class abortDist:
	Id=None
	abortNum={}

def getAbortDist(filePrefix,nthreads):
	i =0
	threadAbortDist=[]
	while i < nthreads:
		filename = filePrefix + str(i)
		fp=open(filename)
		lines=fp.readlines()
		fp.close()
		words=lines[0].split(';')
		del words[-1]
		abStat=abortDist()
		abStat.Id = i
		for word in words:
			chits=word.split('@')
			aborts=int(chits[1])
			if aborts in abStat.abortNum:
				abStat.abortNum[aborts]+=1
			else:
				abStat.abortNum[aborts]=1
		threadAbortDist.append(abStat)
		i+=1
	for thread in threadAbortDist:
		print("thread%d "%(thread.Id)),
		for w in sorted(thread.abortNum, key=thread.abortNum.get, reverse=True):
			print("%d:%d"%(w,thread.abortNum[w])),
		print

class stats:
	threadId=None
	tavg=0.0
	tstd=0.0
	freq=0
def getTimeStats(filePrefix,nthreads):
	i=0
	threadStats=[]
	while i < nthreads:
		filename = filePrefix + str(i)
		fp=open(filename)
		lines=fp.readlines()
		fp.close()
		words=lines[0].split(';')
		del words[-1]
		words.pop(0)#remove the first read 
		tstat=stats()
		tstat.threadId = i
		for word in words:
			tstat.tavg += float(word)
			tstat.freq +=1
		tstat.tavg/=tstat.freq
		for word in words:
			tstat.tstd +=(float(word) - tstat.tavg)**2
		tstat.tstd = (tstat.tstd/tstat.freq)**0.5
		threadStats.append(tstat)
		i+=1
	for thread in threadStats:
		print("thread%d %f %f"%(thread.threadId,thread.tavg,thread.tstd))

class tstate:
	st=None
	frequency=None
class states:
	state=None
	transition=None

def genStates(filename):
	fp= open(filename)
	lines=fp.readlines()
        fp.close()
	words=lines[0].split(';')
	numwords = len(words)
	i =0
	newstates = {}
	while i+1 < numwords:
		cur = words[i]
		nex = words[i+1]
		if cur in newstates:
			v = newstates[cur]
			if nex in v.transition:
				x = v.transition[nex]
				x.frequency += 1
			else:
				rstate = tstate()
				rstate.st = nex
				rstate.frequency = 1
				v.transition[nex] = rstate
		else:
			tmpstate = states()
			tmpstate.state = cur
			tmpstate.transition={}
			ntstate = tstate()
			ntstate.st = nex
			ntstate.frequency =1 
			tmpstate.transition[nex]=ntstate
			newstates[cur] = tmpstate
		i += 1
	return newstates


def main():
        nThreads = int(sys.argv[1])
        nRuns = int(sys.argv[2])
        option = sys.argv[3]
        freq=int(sys.argv[4])

        if option == "mcmc_data":
                newstates = genStates("mcmc_data")
		fp=open("state_data","w")
		for s in newstates.values():
			fp.write("%s-"%s.state),
			for ns in sorted(s.transition.values(),key=operator.attrgetter('frequency'), reverse=True):
				fp.write(" %s:%d"%(ns.st,ns.frequency))
			fp.write("\n")

	elif option == "ND_mcmc" or option == "ND_only":
		newstates = genStates("mcmc_monitor_ND")
		print("NumStates %d"%len(newstates))
		getAbortDist("abortsThread",nThreads)
	else:
		getTimeStats("timeAna",nThreads)
		


if __name__ == "__main__":
	main()
		
	
			
		
	

