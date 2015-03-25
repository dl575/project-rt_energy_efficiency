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
