#!/usr/bin/python
import os
import sys
sys.path.extend(['../'])
sys.path.extend(['/home/odroid/project-rt_energy_efficiency/lib'])
import parse_lib

if __name__ == "__main__":
    if len(sys.argv) < 3 or (sys.argv[1] != "big" and sys.argv[1] != "little"):
        print "usage: plot_both.py {big|little} benchmark [base_dir]"
        exit(0)
    else:
        big_little=sys.argv[1]
        benchmarks=sys.argv[2]
        if len(sys.argv) >= 4:
            cur_path=sys.argv[3]
        else: 
            cur_path=os.getcwd()

    sweep=100
    bench = benchmarks+"-freq_sweep"

    global_moment = parse_lib.parse_int(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/output_power.txt", "moment : ([0-9\.]+) us")
    global_power = parse_lib.parse_float(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/output_power.txt", big_little+" core power : ([0-9\.]+)W")


    # 1. read prediction or cvx file -> freq -> time around
    parse_start = None    
    
    #print len(frequency)
    #print len(moment_start)
    #print len(moment_end)

    # 2. read output_power.txt -> from time around -> find average power
    
    # 3. make table for (f,p)_{bench}_{little/big}
    if big_little == "little":
        max_freq = 1400000
    elif big_little == "big":
        max_freq = 2000000
    powers = []
    f = max_freq
    power_sum = 0.0
    cnt = 0
    s = 0
    e = 0
    saved_end_index = 0
    for f in range(max_freq, 200000-1, -100000):
        power_sum = 0.0
        cnt = 0
        moment_start = parse_lib.parse_int(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/"+str(f), "moment_start_0 : ([0-9\.]+) us", start=parse_start)
        moment_end = parse_lib.parse_int(cur_path+"/"+big_little+"/"+benchmarks+"/"+bench+"/"+str(f), "moment_end : ([0-9\.]+) us", start=parse_start)
        for i in xrange(0, len(moment_start)):
            s = moment_start[i]
            e = moment_end[i]
            for k in xrange(saved_end_index, len(global_moment)):
                if global_moment[k] > s :
                    power_sum += global_power[k]
                    cnt +=1
                if global_moment[k] > e :
                    power_sum -= global_power[k]
                    saved_end_index = k
                    break
        powers.append(power_sum/cnt)

    #print power array by C style
    print "#if "+benchmarks
    print "\tfloat power_"+big_little+"_"+"["+str(len(powers))+"] = {",
    for i in reversed(xrange(0,len(powers))):
        print "%.3f"%powers[i],
        if i != 0:
            print ", ",
    print "};"
    print "#endif"
