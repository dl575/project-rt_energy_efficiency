#!/usr/bin/python
import os
import sys
sys.path.extend(['../'])
import tsg_plot
import math
sys.path.extend(['/home/odroid/project-rt_energy_efficiency/lib'])
import parse_lib
import matplotlib.pyplot as plt

# Set up plotting options
opts = tsg_plot.PlotOptions()

def plot(data, labels, kk):
    opts.labels = labels

    attribute_dict = \
        {
            'bar_width' : 0.7,
            'figsize' : (7.0, 2),
            'fontsize' : 4,
            'labels_fontsize' : 4,
            'colors' : ["#a6cee3", "#1f78b4", "#b2df8a", "#33a02c", "#fb9a99", "#e31a1c", "#ff7f00", "#cab2d6", "#6a3d9a", "#ffff99", "#b15928", "#fdbf6f"],
            'paper_mode' : True,
            'show' : False,
            'paper_mode' : True,
            'legend_ncol' : 2,
            'rotate_labels' : True,
            'rotate_labels_angle' : -45,
            'num_rows' : 1,
            'num_cols' : 3
        }

    opts.xlabel = big_little
    if kk == 0 :
        opts.ylabel = "energy [%]"
        opts.legend_enabled = False
    elif kk == 1 :
        opts.ylabel = "deadline_misses [%]"
        opts.legend_enabled = True
    elif kk == 2 :
        opts.ylabel = "big usages [%]"
        opts.legend_enabled = False
        opts.file_name = ""+big_little+"_"+benchmarks+".pdf"
    opts.data = data
    for name, value in attribute_dict.iteritems():
        setattr( opts, name, value )

    # Plot
    tsg_plot.add_plot( opts )

def parse(kk):
    # Clear output file
    f=open('parsed_data'+str(kk)+'.csv', 'w')
    f.close()    
    f2=open('_parsed_data'+str(kk)+'_'+big_little+'.csv', 'w')
    f2.close()    
    # Write out governors
    for idx, val in enumerate(governor_files):
        f=open('parsed_data'+str(kk)+'.csv', 'a')
        f.write(','+ val)
        f.close()
        f2=open('_parsed_data'+str(kk)+'_'+big_little+'.csv', 'a')
        f2.write(','+ val)
        f2.close()
        
    for bench in benchmarks_list:
        print "---------"+bench+"----------"
        #get global_moment and global_power
        global_moment = parse_lib.parse_int(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/output_power.txt", "moment : ([0-9\.]+) us")
        global_power = parse_lib.parse_float(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/output_power.txt", big_little+" core power : ([0-9\.]+)W")
        overhead_index=0;
        saved_end_index=0
        total_energy_performance = None
        parse_start = None
        #get deadline from first file
        deadline = int(parse_lib.find(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/"+"performance", "deadline time : ([0-9\.]+) us"))

        # Write out benchmark name
        f=open('parsed_data'+str(kk)+'.csv', 'a')
        f.write('\n'+bench+'('+str(deadline)+')');
        f2=open('_parsed_data'+str(kk)+'_'+big_little+'.csv', 'a')
        f2.write('\n'+bench+'('+str(deadline)+')');

        # For each governor
        for jj in governor_files:
            #prediction > prediction_with_overhead and prediction_wo_overhead
            total_energy=0.0

            # Deadline misses
            if kk == 1:
                exec_time = parse_lib.parse_int(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/"+jj, "time [0-9]+ = ([0-9]+) us", start=parse_start)
                # Linux governors, prediction w/o overhead, cvx w/o overhead
                if ("performance" in jj) | ("interactive" in jj) | ("conservative" in jj) | ("ondemand" in jj) | ("powersave" in jj) | ("prediction_wo_overhead" in jj) | ("cvx_wo_overhead" in jj) :
                    #get deadline misses, margin 0%
                    deadline_miss = parse_lib.deadline_misses(exec_time, deadline, margin=1.0)
                # proactive
                elif ( "proactive" in jj):
                    jumps = parse_lib.parse_int(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/"+jj, "jump = ([0-9]+)", start=parse_start)
                    groups = parse_lib.parse_int(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/"+jj, "group of ([0-9]+)", start=parse_start)
                    deadline_miss = parse_lib.deadline_misses_proactive(exec_time, deadline, jumps, groups, margin=1.0)
                # Prediction with overhead, oracle, pid
                else:
                    total_time_before_delay=[]
                    slice_time = parse_lib.parse_int(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/"+jj, "time_slice [0-9]+ = ([0-9]+) us", start=parse_start)
                    dvfs_time = parse_lib.parse_int(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/"+jj, "time_dvfs [0-9]+ = ([0-9]+) us", start=parse_start)
                    # oracle : no slice time, but include dvfs_time
                    if (jj == "oracle"):
                        slice_time = dvfs_time
                    for n in xrange(0, len(exec_time)):
                        if (jj == "oracle"):
                            slice_time[n] = 0
                        total_time_before_delay.append(exec_time[n] + slice_time[n] + dvfs_time[n])
                    #get deadline misses, margin 0%
                    deadline_miss = parse_lib.deadline_misses(total_time_before_delay, deadline, margin=1.0)
                # 0: prediction_with_overhead, 1: predcition_wo_overhead
                print jj
                #write deadline miss rate
                print deadline_miss
                f.write(","+"%f"%(deadline_miss))
                f2.write(","+"%f"%(deadline_miss))
            
            # Tardiness
            elif kk == 2:
                print "big core usages"
                print jj
                if big_little == "big" :
                    f.write(","+"%f"%(100))
                    f2.write(","+"%f"%(100))
                    print "100"
                elif big_little == "little" :
                    f.write(","+"%f"%(0))
                    f2.write(","+"%f"%(0))
                    print "0"
            # Power
            elif kk == 0:
                # Linux governors, prediction with overhead, cvx with overhead, pid
                if ("performance" in jj) | ("interactive" in jj) | ("conservative" in jj) | ("ondemand" in jj) | ("powersave" in jj) | ("prediction_with_overhead" in jj) | ("cvx_with_overhead" in jj) | ("proactive_with_overhead" in jj) | ("pid" in jj) :
                  moment_start = parse_lib.parse_int(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/"+jj, "moment_start_0 : ([0-9\.]+) us", start=parse_start)
                # Prediction w/o overhead, oracle
                else:
                  moment_start = parse_lib.parse_int(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/"+jj, "moment_start_1 : ([0-9\.]+) us", start=parse_start)
                #get end
                moment_end = parse_lib.parse_int(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/"+jj, "moment_end : ([0-9\.]+) us", start=parse_start)

                #size comparison
                if (len(moment_start) != len(moment_end)):
                    print "size of moment_start != size of moment_end"
                    exit(0)
                ################################ calculate power start ###############################
                # For each job,
                for iii in xrange(0, len(moment_start)):
                    #get start_index and end_index
                    for k in xrange(saved_end_index, len(global_moment)):
                        if( global_moment[k] > moment_start[iii]):
                            start_index = k-1;
                            break
                    for k in xrange(start_index, len(global_moment)):
                        if( global_moment[k] > moment_end[iii]):
                            end_index = k+1;
                            # Keep saved_end_index at previous value for second pass at calculating prediction power
                            if (jj != "prediction"):
                                saved_end_index = end_index;
                            break
                    #error check
                    if (k > len(global_moment)-1):
                        print "index error"
                        exit(0)
                    for i in range(start_index, end_index):
                        #unit = W * us = uJ
                        if start_index == end_index:
                            total_energy+=(global_power[i]+global_power[i+1])/2*(moment-end[iii]-moment_start[iii])
                            break
                        if i == start_index:
                            total_energy+=(global_power[i]+global_power[i+1])/2*(global_moment[i+1]-moment_start[iii])
                        elif i == end_index-1:
                            total_energy+=(global_power[i]+global_power[i+1])/2*(global_moment[i+1]-moment_end[iii])
                            break
                        else:
                            total_energy+=(global_power[i]+global_power[i+1])/2*(global_moment[i+1]-global_moment[i])
                        print "power"+str(i)+" : "+str(global_power[i])
                    print "energy"+str(iii)+" : "+str(total_energy)
                ################################ calculate power end ###############################
                # 0: prediction_with_overhead, 1: predcition_wo_overhead
                print jj

                #save total_energy of performance governor 
                if not total_energy_performance:
                    total_energy_performance=total_energy

                #write total energy
                print total_energy
                f.write(","+"%f"%(total_energy/total_energy_performance*100))
                f2.write(","+"%f"%(total_energy))
                print (total_energy/total_energy_performance*100)
        f.close()    
        f2.close()    

if __name__ == "__main__":
    if len(sys.argv) < 3 or (sys.argv[1] != "big" and sys.argv[1] != "little" and sys.argv[1] != "hetero"):
        print "usage: plot_both.py {big|little|hetero} benchmark [base_dir]"
        exit(0)
    else:
        big_little=sys.argv[1]
        benchmarks=sys.argv[2]
        if len(sys.argv) >= 4:
            cur_path=sys.argv[3]
        else: 
            cur_path=os.getcwd()

    benchmarks_list=[]
    #sweep_list=[20, 40, 60, 80, 100, 120, 140, 160, 180]
    #sweep_list=[20, 100, 180]
    sweep_list = [60, 80, 100]

    if(big_little == "big"):
        for sweep in sweep_list:
            benchmarks_list.append(benchmarks+"-"+str(sweep))
    elif(big_little == "little"):
        for sweep in sweep_list:
            benchmarks_list.append(benchmarks+"-"+str(sweep))

    print benchmarks_list
    #######################################################
    #  IMPORTANT : THIS SHOULD BE IN ORDER...
    #######################################################
    # Power calculation assumes that experiments were run in this order
    if "uzbl" in benchmarks_list:
        governor_files = ["performance", "interactive", "prediction_with_overhead", "prediction_wo_overhead", "cvx_with_overhead", "cvx_wo_overhead", "pid"]
    elif "xpilot_slice" in benchmarks_list:
        governor_files = ["performance", "interactive", "prediction_with_overhead", "prediction_wo_overhead", "cvx_with_overhead", "cvx_wo_overhead", "pid"]
    else:
        #governor_files = ["performance", "interactive", "cvx_with_overhead", "cvx_wo_overhead", "oracle", "pid", "proactive_with_overhead-W3", "proactive_wo_overhead-W3", "proactive_with_overhead-W5", "proactive_wo_overhead-W5", "proactive_with_overhead-W10", "proactive_wo_overhead-W10", "proactive_with_overhead-W20", "proactive_wo_overhead-W20"]
        governor_files = ["cvx_with_overhead", "proactive_with_overhead-W5"]

    # For [energy, deadline_misses, tardiness]
    for kk in range(3):
        # Parse data and write to csv file
        parse(kk)

        # Read in csv files
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
        labels = [cat, subcat]

        # Plot results
        plot(data, labels, kk)
