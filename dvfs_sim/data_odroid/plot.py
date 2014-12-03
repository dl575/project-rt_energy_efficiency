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
big_little=("little", "big")
sample_frame=700;

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
#        'file_name' : 'power_odroid.pdf',
        'paper_mode' : True,
        'figsize' : (3.5, 2.5),
        'ylabel' : 'Power (W)',
        'legend_ncol' : 2,
        'rotate_labels' : True,
        'rotate_labels_angle' : -45,
        'num_rows' : 1,
        'num_cols' : 2
    }


for k in xrange(0, len(big_little)):
    benchmarks = [f for f in os.listdir(cur_path+"/"+big_little[k]) ]
    governor_files = [f for f in os.listdir(cur_path+"/"+big_little[k]+"/stringsearch") ]

    f=open('parsed_data.csv', 'w')
    f.close()    
    for i in governor_files:
        f=open('parsed_data.csv', 'a')
        f.write(','+i)
        f.close()    
        
    for j in benchmarks:
        f=open('parsed_data.csv', 'a')
        f.write('\n'+j);
    
        for i in governor_files:

            total_energy=0
            power = parse_lib.parse(cur_path+"/"+big_little[k]+"/"+j+"/"+i, "power : ([0-9\.]+)")
            power = [float(x) for x in power]
            power=power[0:sample_frame]
            time = parse_lib.parse(cur_path+"/"+big_little[k]+"/"+j+"/"+i, "time [0-9]+ = ([0-9]+) us")
            time = [int(x) for x in time]
            for i in xrange(1, len(time)):
                time[i]+=time[i-1]
            if len(time) > 0 :
                time[0]=0
                time=time[0:sample_frame]
                for i in range(0, sample_frame-1):
                    total_energy+=(time[i+1]-time[i])*power[i]
            f.write(","+"%f"%(total_energy/1000000))
    
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

    opts.xlabel = big_little[k]
    if opts.plot_idx == 1 : 
        opts.legend_enabled = True
    else:
        opts.legend_enabled = False
        opts.file_name = "power_odroid.pdf"

    opts.data = data
    opts.labels = [cat, subcat]
    for name, value in attribute_dict.iteritems():
        setattr( opts, name, value )

    # Plot
    tsg_plot.add_plot( opts )
