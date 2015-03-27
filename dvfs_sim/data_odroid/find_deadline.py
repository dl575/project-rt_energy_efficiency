#!/usr/bin/python
import sys
import re
"""
Parse based on the passed regular expression.
"""
def parse(filename, regex):
    f = open(filename, 'r')
    data = []
    for line in f:
        res = re.search(regex, line)
        if res:
            data.append(res.group(1))
    f.close()
    return data

max_time = 0;
avg_time = 0;
time = parse(sys.argv[1], "time [0-9]+ = ([0-9]+) us")
time = [int(x) for x in time]
for i in xrange(0, len(time)):
    avg_time += time[i]
    if(time[i]>max_time):
        max_time = time[i]
avg_time = avg_time / len(time)

print "deadline : "
print max_time

max_slice_time = 0;
avg_slice_time = 0;
time_slice = parse(sys.argv[2], "time_slice [0-9]+ = ([0-9]+) us")
time_slice = [int(x) for x in time_slice]
for i in xrange(0, len(time_slice)):
    avg_slice_time += time_slice[i]
    if(time_slice[i]>max_slice_time):
        max_slice_time = time_slice[i]
avg_slice_time = avg_slice_time / len(time_slice)

print "deadline_slice : "
print max_slice_time

max_dvfs_time = 0;
avg_dvfs_time = 0;
time_dvfs = parse(sys.argv[2], "time_dvfs [0-9]+ = ([0-9]+) us")
time_dvfs = [int(x) for x in time_dvfs]
for i in xrange(0, len(time_dvfs)):
    avg_dvfs_time += time_dvfs[i]
    if(time_dvfs[i]>max_dvfs_time):
        max_dvfs_time = time_dvfs[i]
avg_dvfs_time = avg_dvfs_time / len(time_dvfs)

print "deadline_dvfs : "
print max_dvfs_time

max_overhead_time = 0;

for i in xrange(0, len(time_dvfs)):
    if( (time_slice[i]+time_dvfs[i]) > max_overhead_time):
        max_overhead_time = time_slice[i] + time_dvfs[i]

print "deadline_overhead(dvfs+slice) : "
print max_overhead_time

print "--------------------"

print "#define OVERHEAD_TIME "+str(max_slice_time+max_dvfs_time)+" //overhead deadline"
print "#define DEADLINE_TIME "+str(max_time)+" + OVERHEAD_TIME //deadline"
print "#define MAX_DVFS_TIME "+str(max_dvfs_time)+" //max dvfs time"
print "#define GET_PREDICT 0 //to get prediction equation"
print "#define GET_OVERHEAD 0 //to get overhead deadline"

print "--------------------"

print "#define OVERHEAD_TIME "+str(max_overhead_time)+" //overhead deadline"
print "#define DEADLINE_TIME "+str(max_time)+" + OVERHEAD_TIME //deadline"
print "#define MAX_DVFS_TIME "+str(max_dvfs_time)+" //max dvfs time"
print "#define AVG_DVFS_TIME "+str(avg_dvfs_time)+" //average dvfs time"
print "#define GET_PREDICT 0 //to get prediction equation"
print "#define GET_OVERHEAD 0 //to get overhead deadline"

print "--------------------"

print "#define OVERHEAD_TIME "+str(max_overhead_time)+" //overhead deadline"
print "#define AVG_OVERHEAD_TIME "+str(avg_slice_time+avg_dvfs_time)+" //avg overhead deadline"
print "#define DEADLINE_TIME "+str(avg_time)+" + AVG_OVERHEAD_TIME //avg deadline"
print "#define MAX_DVFS_TIME "+str(max_dvfs_time)+" //max dvfs time"
print "#define AVG_DVFS_TIME "+str(avg_dvfs_time)+" //average dvfs time"
print "#define GET_PREDICT 0 //to get prediction equation"
print "#define GET_OVERHEAD 0 //to get overhead deadline"
 #define OVERHEAD_TIME 43925 //overhead deadline
 #if OVERHEAD_EN
 #define DEADLINE_TIME 52905 + OVERHEAD_TIME //big with overhead
 #else
 #define DEADLINE_TIME 52905 //big without overhead 
 #endif

