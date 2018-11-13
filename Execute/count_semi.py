#!/usr/bin/python

import sys

filename=sys.argv[1]
fp=open(filename)
lines=fp.readlines()
fp.close()
count=0
for line in lines:
	words=line.split(';')
	count+=len(words)

print count
