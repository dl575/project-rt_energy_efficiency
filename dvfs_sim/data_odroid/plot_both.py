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
#if(len(sys.argv)!=2):
#    print "argument is needed"
#    exit(0)
#if((sys.argv[1] != "big_with_overhead") & (sys.argv[1] != "big_wo_overhead")):
#    print "argument big_with_overhead or big_wo_overhead"
#    exit(0)

big_little="big"

#benchmarks = ["stringsearch", "xpilot_slice", "sha", "rijndael", "julius_slice", "2048_slice", "curseofwar_slice"]#, "average"]
#sample_frame=[1300          , 2260          , 100  , 200       , 50            , 50          , 2000]
#benchmarks =   ["sha", "rijndael", "stringsearch", "xpilot_slice", "julius_slice"]
#benchmarks =   ["stringsearch", "sha", "rijndael", "xpilot_slice", "julius_slice", "2048_slice", "curseofwar_slice"]
benchmarks =   ["stringsearch", "sha", "rijndael", "xpilot_slice",  "2048_slice", "curseofwar_slice"]
#sample_frame = [100]

#governor_files = ["performance", "prediction_with_overhead", "prediction_wo_overhead", "powersave", "conservative", "interactive", "ondemand"]
#IMPORTANT : THIS SHOULD BE IN ORDER...
governor_files = ["performance", "interactive", "conservative", "ondemand", "powersave", "prediction_with_overhead", "prediction_wo_overhead"]

#variables
total_energy=0.0;
deadline_margin=1.1;

cur_path=os.getcwd()

# Set up plotting options
opts = tsg_plot.PlotOptions()

attribute_dict = \
    {
        'bar_width' : 0.7,
        'figsize' : (7.5, 2),
        'fontsize' : 7,
        'labels_fontsize' : 7,
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

for kk in range(0, 2):
    f=open('parsed_data'+str(kk)+'.csv', 'w')
    f.close()    
    for ii in governor_files:
        f=open('parsed_data'+str(kk)+'.csv', 'a')
        f.write(','+ii)
        f.close()    
        
    for bench in benchmarks:
        print "---------"+bench+"----------"
        #get global_moment and global_power
        global_moment = parse_lib.parse(cur_path+"/"+big_little+"/"+bench+"/output_power.txt", "moment : ([0-9\.]+) us")
        global_moment = [int(x) for x in global_moment]
        global_power = parse_lib.parse(cur_path+"/"+big_little+"/"+bench+"/output_power.txt", "big core power : ([0-9\.]+)W")
        global_power = [float(x) for x in global_power]
        saved_end_index=0
        cnt_performance=0
        f=open('parsed_data'+str(kk)+'.csv', 'a')
        f.write('\n'+bench);
        for jj in governor_files:
            print jj
            #if cnt_performance == 1:
            #    saved_end_index = 0
            #    cnt_performance = 2
            total_energy=0.0
            #get deadline, margin 10%
            deadline_miss=0.0
            deadline_margin=1.1
            deadline_time = parse_lib.parse(cur_path+"/"+big_little+"/"+bench+"/"+jj, "deadline time : ([0-9\.]+) us")
            deadline_time = [int(x) for x in deadline_time]
            deadline = deadline_time[0]

            #get deadline miss rate
            total_time = parse_lib.parse(cur_path+"/"+big_little+"/"+bench+"/"+jj, "total_time [0-9]+ = ([0-9]+) us")
            total_time = [int(x) for x in total_time]
            for i in xrange(0, len(total_time)):
                if(total_time[i]>deadline*deadline_margin):
                    deadline_miss=deadline_miss+1
            deadline_miss=deadline_miss/len(total_time)*100

            #get start
            moment_start = parse_lib.parse(cur_path+"/"+big_little+"/"+bench+"/"+jj, "moment_start : ([0-9\.]+) us")
            moment_start = [int(x) for x in moment_start]
            #start = moment_start[0]

            #get end
            moment_end = parse_lib.parse(cur_path+"/"+big_little+"/"+bench+"/"+jj, "moment_end : ([0-9\.]+) us")
            moment_end = [int(x) for x in moment_end]
            #end = moment_end[len(moment_end)-1]

            #size comparison
            if (len(moment_start) != len(moment_end)):
                print "size of moment_start != size of moment_end"
                exit(0)

            for iii in xrange(0, len(moment_start)):
                #get start_index and end_index
                for k in xrange(saved_end_index, len(global_moment)):
                    if( global_moment[k] > moment_start[iii]):
                        start_index = k-1;
                        break;
                for k in xrange(saved_end_index, len(global_moment)):
                    if( global_moment[k] > moment_end[iii]):
                        end_index = k+1;
                        saved_end_index = end_index;
                        break;

                #error check
                if (k > len(global_moment)-1):
                    print "index error"
                    exit(0)

                #power calculation (start_index <= i < end_index)
                for i in range(start_index, end_index):
                    #unit = W * us = uJ
                    if start_index == end_index:
                        total_energy+=(global_power[i]+global_power[i+1])/2*(moment-end[iii]-moment_start[iii])
                        break;
                    if i == start_index:
                        total_energy+=(global_power[i]+global_power[i+1])/2*(global_moment[i+1]-moment_start[iii])
                    elif i == end_index-1:
                        total_energy+=(global_power[i]+global_power[i+1])/2*(global_moment[i+1]-moment_end[iii])
                        break;
                    else:
                        total_energy+=(global_power[i]+global_power[i+1])/2*(global_moment[i+1]-global_moment[i])

            #save total_energy of performance governor 
            if cnt_performance == 0:
                total_energy_performance=total_energy
                cnt_performance = 1

            #write total energy
            if kk == 0 :
                f.write(","+"%f"%(total_energy/total_energy_performance*100))
                print (total_energy/total_energy_performance*100)
            #write deadline miss rate
            elif kk == 1 :
                print deadline_miss
                f.write(","+"%f"%(deadline_miss))
        f.close()    

    # Parse data
    f=open('parsed_data'+str(kk)+'.csv', 'r')
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
    if kk == 0 :
        opts.ylabel = "energy [%]"
        opts.legend_enabled = True
    else :
        opts.ylabel = "deadline_misses [%]"
        opts.legend_enabled = False
        opts.file_name = "governors_both.pdf"
    opts.data = data
    opts.labels = [cat, subcat]
    for name, value in attribute_dict.iteritems():
        setattr( opts, name, value )

    # Plot
    tsg_plot.add_plot( opts )


