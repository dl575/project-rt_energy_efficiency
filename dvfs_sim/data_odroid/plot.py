#!/usr/bin/python
import os
import sys
sys.path.extend(['../'])
import tsg_plot
import math
sys.path.extend(['/home/odroid/project-rt_energy_efficiency/lib'])
import parse_lib
import matplotlib.pyplot as plt

#manually set below
#big_little='little'
big_little="big"

#             freeciv_slice, xpilot_slice  stringsearch
sample_frame=[700          , 1500        , 1300         ];

#variables
total_energy=0;

cur_path=os.getcwd()

# Set up plotting options
opts = tsg_plot.PlotOptions()

attribute_dict = \
    {
        'bar_width' : 0.7,
        'figsize' : (7.5, 2),
        'fontsize' : 8,
        'labels_fontsize' : 8,
        'colors' : ["#a6cee3", "#1f78b4", "#b2df8a", "#33a02c", "#fb9a99", "#e31a1c", "#ff7f00", "#cab2d6", "#6a3d9a", "#ffff99", "#b15928", "#fdbf6f"],
        'paper_mode' : True,
        'show' : False,
        #'file_name' : 'power_odroid.pdf',
        'paper_mode' : True,
        'figsize' : (3.5, 2.5),
        #'ylabel' : 'Power (W)',
        'legend_ncol' : 2,
        'rotate_labels' : True,
        'rotate_labels_angle' : -45,
        'num_rows' : 1,
        'num_cols' : 2
    }


for k in range(0, 2):
    benchmarks = [f for f in os.listdir(cur_path+"/"+big_little) ]
    governor_files = [f for f in os.listdir(cur_path+"/"+big_little+"/stringsearch") ]
    cnt=0 
    f=open('parsed_data.csv', 'w')
    f.close()    
    for i in governor_files:
        f=open('parsed_data.csv', 'a')
        f.write(','+i)
        f.close()    
        
    for j in benchmarks:
        cnt2=0
        print "------------"
        print j
        f=open('parsed_data.csv', 'a')
        f.write('\n'+j);
        for i in governor_files:
            print i
            total_energy=0.0
            deadline_miss=0.0
            deadline_time = parse_lib.parse(cur_path+"/"+big_little+"/"+j+"/"+i, "deadline time : ([0-9\.]+) us")
            deadline_time = [int(x) for x in deadline_time]
            power = parse_lib.parse(cur_path+"/"+big_little+"/"+j+"/"+i, "big core power : ([0-9\.]+)")
            power = [float(x) for x in power]
            power=power[0:sample_frame[cnt]]
            time = parse_lib.parse(cur_path+"/"+big_little+"/"+j+"/"+i, "time [0-9]+ = ([0-9]+) us")
            time = [int(x) for x in time]
            for i in xrange(1, len(time)):
                if(time[i]>deadline_time[0]):
                    deadline_miss=deadline_miss+1
            deadline_miss=deadline_miss/len(time)*100
            for i in xrange(1, len(time)):
                time[i]+=time[i-1]
            if len(time) > 0 :
                time[0]=0
                time=time[0:sample_frame[cnt]]
                for i in range(0, sample_frame[cnt]-1):
                    total_energy+=(time[i+1]-time[i])*power[i]
            if cnt2 == 0:
                total_energy_performance=total_energy
                cnt2=1
            if k == 0 :
                print total_energy_performance
                f.write(","+"%f"%(total_energy/total_energy_performance*100))
            elif k == 1 :
                f.write(","+"%f"%(deadline_miss))
        cnt=cnt+1
        f.close()    

    # Parse data
    f = open("parsed_data.csv", 'r')
    subcat = []
    subcat = f.readline().strip().split(',')[1:]
    cat = []
    data = []
    for line in f:
        ls = line.split(',')
        cat.append(ls[0])
        data.append([float(x) for x in ls[1:]])
    f.close()

    opts.xlabel = big_little
    if k == 0 :
        opts.ylabel = "energy [%]"
        opts.legend_enabled = True
    else :
        opts.ylabel = "deadline_misses [%]"
        opts.legend_enabled = False
        opts.file_name = "governors.pdf"
    opts.data = data
    opts.labels = [cat, subcat]
    for name, value in attribute_dict.iteritems():
        setattr( opts, name, value )

    # Plot
    tsg_plot.add_plot( opts )


