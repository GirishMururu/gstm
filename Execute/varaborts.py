#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import numpy as np
import matplotlib.pyplot as plt

def main():
        plt.style.use('ggplot')
        fp = open(sys.argv[2])
        lines = fp.readlines()
        fp.close()
        numthreads = int(sys.argv[1])
        avgabort = [0.0 for i in range(numthreads)]
        thread_sets = lines[0].split(";")
        count = len(thread_sets) -1
        for thread_set in thread_sets:
            thread_aborts = thread_set.split(",")
	    if len(thread_aborts) < numthreads:
		continue
            i = 0
            while i < numthreads:
                avgabort[i] += int(thread_aborts[i])
                i += 1
        #print("\n Calculating Average aborts %d values"%count)
        for idx,tabort in enumerate(avgabort):
            avgabort[idx] = tabort/count

        varabort = [0.0 for i in range(numthreads)]
        for thread_set in thread_sets:
            thread_aborts = thread_set.split(",")
	    if len(thread_aborts) < numthreads:
		continue
            i = 0
            while i < numthreads:
                varabort[i] += (int(thread_aborts[i]) - avgabort[i])**2/count
                i += 1

        for idx,var in enumerate(varabort):
            varabort[idx] = var **0.5

        Modelabort_avg = [x for x in avgabort]
        Modelabort_std = [x for x in varabort]

        del lines[:]
        del thread_sets[:]

        avgabort[:] = [0.0 for i in avgabort]
        varabort[:] = [0.0 for i in varabort]

        fp = open(sys.argv[3])
        lines = fp.readlines()
        fp.close()
        thread_sets = lines[0].split(";")
        count = len(thread_sets) -1
        for thread_set in thread_sets:
            thread_aborts = thread_set.split(",")
            if len(thread_aborts) < numthreads:
                continue
            i = 0
            while i < numthreads:
                avgabort[i] += int(thread_aborts[i])
                i += 1
        #print("\n Calculating Average aborts %d values"%count)
        for idx,tabort in enumerate(avgabort):
            avgabort[idx] = tabort/count

        for thread_set in thread_sets:
            thread_aborts = thread_set.split(",")
            if len(thread_aborts) < numthreads:
                continue
            i = 0
            while i < numthreads:
                varabort[i] += (int(thread_aborts[i]) - avgabort[i])**2/count
                i += 1

        for idx,var in enumerate(varabort):
            varabort[idx] = var **0.5

        Origabort_avg = [x for x in avgabort]
        Origabort_std = [x for x in varabort]


        #Calculate the percentage improvement of aborts and abort.dev

        avgimprov = [0.0 for i in range(numthreads)]
        varimprov = [0.0 for i in range(numthreads)]
        i = 0
        while i < numthreads:
            avgimprov[i] = (Origabort_avg[i] - Modelabort_avg[i])*100/Origabort_avg[i]
            varimprov[i] = (Origabort_std[i] - Modelabort_std[i])*100/Origabort_std[i]
            i += 1

        fp = open("abort.stats","w+")
        numinst = 0
        avgonallthreads = 0.0
        for avg in avgimprov:
            avgonallthreads += avg
            numinst += 1

        avgonallthreads = avgonallthreads/numinst

        numinst = 0
        varonallthreads = 0.0
        for var in varimprov:
            varonallthreads += var
            numinst += 1

        varonallthreads = varonallthreads/numinst

        fp.write("%f\n%f\n"%(avgonallthreads,varonallthreads))
        fp.close()


        # These are the "Tableau 20" colors as RGB.
        tableau20 = [(31, 119, 180), (174, 199, 232), (255, 127, 14), (255, 187, 120),
             (44, 160, 44), (152, 223, 138), (214, 39, 40), (255, 152, 150),
             (148, 103, 189), (197, 176, 213), (140, 86, 75), (196, 156, 148),
             (227, 119, 194), (247, 182, 210), (127, 127, 127), (199, 199, 199),
             (188, 189, 34), (219, 219, 141), (23, 190, 207), (158, 218, 229)]

        tableauCB = [(0,107,164),(255,128,14)]
        # Scale the RGB values to the [0, 1] range, which is the format matplotlib accepts.
        for i in range(len(tableau20)):
            r, g, b = tableau20[i]
            tableau20[i] = (r / 255., g / 255., b / 255.)

        for i,colorcomp in enumerate(tableauCB):
            r, g, b =  colorcomp
            tableauCB[i] = (r/255., g/255., b/255.)

        fig = plt.figure()
        ax = fig.add_subplot(111)

        index = np.arange(numthreads)
        bar_width = 0.35

        opacity = 0.5
        error_config = {'ecolor': '0.3'}

        rects1 = ax.bar(index, Modelabort_avg, bar_width,
                 alpha=opacity,
                 color= tableauCB[0],
                 linewidth=0,
                 edgecolor ="none",# tableauCB[0],
                 yerr= Modelabort_std,
                 error_kw=dict(elinewidth=2,ecolor=tableauCB[1]))
                 #label='Average Aborts improvement')
                 #label='G-stm')

        rects2 = ax.bar(index + bar_width, Origabort_avg, bar_width,
                 alpha=opacity,
                 color=tableauCB[1],
                 linewidth=0,
                 edgecolor="none",#tableauCB[1],
                 yerr=Origabort_std,
                 hatch = '//',
                 error_kw=dict(elinewidth=2,ecolor=tableauCB[0]))
                 #label='Abort std.dev improvement')
                 #label='Original')

        #plt.xlabel('Threads')
        #plt.ylabel('#Aborts')
        #plt.title('Aborts and its variance per thread')
        xticks = ['Thread'+str(i) for i in range(numthreads)]
        ax.set_xlim(-bar_width, len(index)+bar_width)
        ax.set_ylabel('Aborts',fontsize=10)
        ax.set_title('Change in Aborts per thread',fontsize=10)
        ax.set_xticks(index+bar_width)
        xticknames = ax.set_xticklabels(xticks)
        plt.setp(xticknames, rotation=45, fontsize=10)
        #plt.xticks(index + bar_width, xticks)
        ax.legend((rects1[0],rects2[0]),('G-Stm', 'Original'),fontsize=10)

        for tick in ax.xaxis.get_major_ticks():
            tick.label.set_fontsize(10)
        
        for tick in ax.yaxis.get_major_ticks():
            tick.label.set_fontsize(10)

        plt.tight_layout()
        #plt.show()
        plt.savefig("Abort.pdf")


if __name__ == "__main__":
    main()














