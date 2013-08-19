#!/usr/bin/python

"""
Plot the execution time for each frame.
"""

import re
import sys
import matplotlib.pyplot as plot

if len(sys.argv) not in [2, 3]:
  raise Exception("usage: spec_plot_time.py filename")
if len(sys.argv) == 3:
  title = sys.argv[2]
else:
  title = ''

# Open file
f = open(sys.argv[1], 'r')

times = []
# For each line
for line in f:
  # Use regular expressions to parse
  res = re.search("dlo: timing = ([0-9]+)", line)
  if res:
    time = int(res.group(1))
    times.append(time)

# Plot
fig = plot.figure()
ax1 = plot.subplot(111)
# Line in gray
ax1.plot(times, "#AAAAAA")
# Points in blue
ax1.plot(times, 'b.')
ax1.set_xlabel("Instance")
ax1.set_ylabel("Execution time [us]")
ax1.set_title(title)
plot.show()

# Close files
f.close()
