#!/usr/bin/python
import sys
import os
sys.path.extend(['/home/odroid/project-rt_energy_efficiency/lib'])
import parse_lib
import matplotlib.pyplot as plt

# Default fonts for plots
default_font = {"family" : "Times New Roman",
    "size" : 9}
# Default figsize for viewing
figsize_default = (8.0, 6.0)
# Single column figure size
figsize_singlecol = (3.5, 3.0)
# Double column figure size
figsize_doublecol = (7.0, 3.0)

total_energy=0;
sample_frame=1300;
for j in xrange(1, len(sys.argv)):
    print j
    governor_name=sys.argv[j]
    power = parse_lib.parse(governor_name, "big core power : ([0-9\.]+)")
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

    plt.plot(time, power, label=sys.argv[j]+" ("+"%.2f"%(total_energy/1000000)+"W) ")


#inter,=plt.plot(time1, data1, label=sys.argv[2])
#perf,=plt.plot(time2, data2, label=sys.argv[3])
#title = os.getcwd()
#title = title.split("/benchmarks/")
#plt.title(title[1])
plt.xlabel("time(us)")
plt.ylabel("power(W)")
plt.legend()
plt.show()
plt.savefig("governors.pdf", format="pdf")
#parse_lib.plot_scatter(time, data)
#plot_my_scatter(time, data, time1, data1, time2, data2)

#parse_lib.plot_show()
#parse_lib.plot_to_pdf(governor_name+".pdf")
#data = parse("interactive.log", "A7_end : ([0-9\.]+)")
#data = [float(x) for x in data]
#print data[0:10]
