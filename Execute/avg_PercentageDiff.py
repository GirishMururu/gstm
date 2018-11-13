#!/usr/bin/python

import sys

class stat:
	statName=None
	statValue=None

class BenchStats:
	name=None
	numStats=None
	statArray=None

def fillEachFileStat(filename):
	fp=open(filename)
	lines = fp.readlines()
	fp.close()
	Benches=[]
	i =0
	numLines = len(lines)
	while i < numLines:
		words = lines[i].split()
		if len(words) ==1:
			bench = BenchStats()
			bench.name = words[0]
			bench.numStats = 0
			bench.statArray = []
			i+=1
			while i < numLines and len(lines[i].split()) != 1:
				nextwords=lines[i].split()
				newStat = stat()
				newStat.statName = nextwords[0]
				newStat.statValue = float(nextwords[1])
				#newStat.statValue = float(nextwords[2])
				bench.numStats +=1
				bench.statArray.append(newStat)
				i+=1
			if len(bench.statArray) > 1:
				Benches.append(bench)
	return Benches

	
def getdiff(Benches1,Benches2):
	percentBenches=[]
	numBenches = len(Benches1)
	i =0
	while i < numBenches:
		bench1=Benches1[i]
		bench2=Benches2[i]
		if bench1.name != bench2.name:
			i+=1
			continue
		percentbench = BenchStats()
		percentbench.name = bench1.name
		percentbench.numStats=min(bench1.numStats,bench2.numStats)
		percentbench.statArray=[]
		j=0
		while j < percentbench.numStats:
			percentStat=stat()
			stat1=bench1.statArray[j]
			stat2=bench2.statArray[j]
			percentStat.statName=stat1.statName
			percentStat.statValue=(stat1.statValue-stat2.statValue)*100/stat1.statValue
			percentbench.statArray.append(percentStat)
			j+=1
		percentBenches.append(percentbench)
		i+=1
	return percentBenches

def printCus(Benches):
	for bench in Benches:
		print bench.name
		for istat in bench.statArray:
			print("%s = %f"%(istat.statName,istat.statValue))
def main():
	file1=sys.argv[1]
	file2=sys.argv[2]
	Benches1=fillEachFileStat(file1)
	Benches2=fillEachFileStat(file2)
	percentDiff=getdiff(Benches1,Benches2)
	printCus(percentDiff)
	

if __name__ == "__main__":
	main()
