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

time = parse(sys.argv[1], "time [0-9]+ = ([0-9]+) us")
time = [int(x) for x in time]
for i in xrange(0, len(time)):
    if(time[i]>max_time):
        max_time = time[i]

print "deadline : "
print max_time

max_slice_time = 0;

time = parse(sys.argv[2], "time_slice [0-9]+ = ([0-9]+) us")
time = [int(x) for x in time]
for i in xrange(0, len(time)):
    if(time[i]>max_slice_time):
        max_slice_time = time[i]

print "deadline_slice : "
print max_slice_time

max_dvfs_time = 0;

time = parse(sys.argv[2], "time_dvfs [0-9]+ = ([0-9]+) us")
time = [int(x) for x in time]
for i in xrange(0, len(time)):
    if(time[i]>max_dvfs_time):
        max_dvfs_time = time[i]

print "deadline_dvfs : "
print max_dvfs_time

print "--------------------"

print "#define OVERHEAD_TIME "+str(max_slice_time+max_dvfs_time)+" //overhead deadline"
print "#define DEADLINE_TIME "+str(max_time)+" + OVERHEAD_TIME //deadline"
print "#define MAX_DVFS_TIME "+str(max_dvfs_time)+" //max dvfs time"
print "#define GET_PREDICT 0 //to get prediction equation"

 #define OVERHEAD_TIME 43925 //overhead deadline
 #if OVERHEAD_EN
 #define DEADLINE_TIME 52905 + OVERHEAD_TIME //big with overhead
 #else
 #define DEADLINE_TIME 52905 //big without overhead 
 #endif

