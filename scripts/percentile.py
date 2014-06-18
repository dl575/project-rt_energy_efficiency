#!/usr/bin/python

import re
import numpy 
import sys

if len(sys.argv) != 2:
  raise Exception("usage: percentile.py logfile")

print sys.argv[1]

# Find execution times
times = []
f = open(sys.argv[1], 'r')
for line in f:
  res = re.search("time [0-9]+ = ([0-9]+)", line)
  if res:
    times.append(int(res.group(1)))
f.close()

# Get predictions
predict = []
f = open("test.predict", 'r')
for line in f:
  predict.append(int(line))
f.close()

# Find times predicted as fast
data = zip(predict, times)
fast = []
for (p, t) in data:
  if p == -1:
    fast.append(t)

# Print out percentiles
print "Percentile:"
fastnp = numpy.array(fast)
if len(fastnp) == 0:
  print "No fast frames"
else:
  for p in [50, 90, 95, 99, 100]:
    print "%dth Percentile = " % (p),
    print numpy.percentile(fastnp, p)

