#!/usr/bin/python
import sys
import os
sys.path.extend(['/home/odroid/project-rt_energy_efficiency/lib'])
import parse_lib
import matplotlib.pyplot as plt

#manually set below
big_little=0; #0:little
sample_frame=1300;

#variables
total_energy=0;

cur_path=os.getcwd()
dirs=os.listdir(cur_path+"/little/stringsearch/")

f=open('parsed_data.csv', 'w')
f.close()    

benchmarks = [f for f in os.listdir(cur_path+"/little") ]
governor_files = [f for f in os.listdir(cur_path+"/little/stringsearch") ]
print governor_files
print benchmarks

for i in governor_files:
    print i
    f=open('parsed_data.csv', 'a')
    f.write(',x='+i)
    f.close()    

for j in benchmarks:
#    print file
    f=open('parsed_data.csv', 'a')
    f.write('\n'+j);
    
    for i in governor_files:

        power = parse_lib.parse(cur_path+"/little/stringsearch/"+i, "power : ([0-9\.]+)")
        power = [float(x) for x in power]
        power=power[0:sample_frame]
        time = parse_lib.parse(cur_path+"/little/stringsearch/"+i, "time [0-9]+ = ([0-9]+) us")
        time = [int(x) for x in time]
        for i in xrange(1, len(time)):
            time[i]+=time[i-1]
        time[0]=0
        time=time[0:sample_frame]
        total_energy=0
        for i in range(0, sample_frame-1):
            total_energy+=(time[i+1]-time[i])*power[i]
        f.write(","+"%f"%(total_energy/1000000))
    
    f.close()    



for j in xrange(1, len(sys.argv)):
    print j
    governor_name=sys.argv[j]
    power = parse_lib.parse(governor_name, "power : ([0-9\.]+)")
    power = [float(x) for x in power]
    time = parse_lib.parse(governor_name, "time [0-9]+ = ([0-9]+) us")
    time = [int(x) for x in time]
    for i in xrange(1, len(time)):
        time[i]+=time[i-1]
    time[0]=0
    time=time[0:sample_frame]
    power=power[0:sample_frame]
    total_energy=0
    for i in range(0, sample_frame-1):
        total_energy+=(time[i+1]-time[i])*power[i]
    print total_energy/1000000    
   
    f=open('parsed_data.csv', 'w')
    f.write(',x='+sys.argv[j])
    f.write("AAA")
    f.close()    
 
    plt.subplot(121)             # the first subplot in the first figure
    plt.plot(time, power, label=sys.argv[j]+" ("+"%.2f"%(total_energy/1000000)+"W) ")


#inter,=plt.plot(time1, data1, label=sys.argv[2])
#perf,=plt.plot(time2, data2, label=sys.argv[3])
#title = os.getcwd()
#title = title.split("/benchmarks/")
title="AAA"

plt.title(title)
plt.xlabel("time(us)")
plt.ylabel("power(W)")
plt.legend(loc = 'best')
plt.show()

#plt.savefig("governors.pdf", format="pdf")


#parse_lib.plot_scatter(time, data)
#plot_my_scatter(time, data, time1, data1, time2, data2)

#parse_lib.plot_show()
#parse_lib.plot_to_pdf(governor_name+".pdf")
#data = parse("interactive.log", "A7_end : ([0-9\.]+)")
#data = [float(x) for x in data]
#print data[0:10]
