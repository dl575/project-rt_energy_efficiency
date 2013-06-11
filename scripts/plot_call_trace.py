#!/usr/bin/python

"""
Takes results of process_call_trace and plots.

usage: plot_call_trace.py calltrace
"""

import math
import sys
import matplotlib.pyplot as plot
import re

if len(sys.argv) != 2:
  raise Exception(__doc__)
filename = sys.argv[1]

# Parse call trace file
f = open(filename, 'r')

calldepths = []
calltrace = []
for line in f:
  res = re.search("Call depth: ([0-9]+)", line)
  if res:
    calldepths.append(int(res.group(1)))
  res = re.search("Hash dict: ({.+})", line)
  if res:
    calltrace.append(eval(res.group(1)))

f.close()

# Number of subplots needed
num_calldepths = len(calltrace)
sp_side = math.ceil(math.sqrt(num_calldepths))
sp = int(100*sp_side + 10*sp_side)

# Plot
fig = plot.figure()

for (i, c) in enumerate(calltrace):
  ax1 = plot.subplot(sp + i + 1)
  for (h, times) in enumerate(c.itervalues()):
    ax1.scatter([h for x in range(len(times))], times)
  ax1.set_title("Call Depth %d" % calldepths[i])

plot.show()

