#!/usr/bin/python

"""
Plot the execution time for each frame.
"""

import re
import sys
import matplotlib.pyplot as plot

if len(sys.argv) != 2:
  raise Exception("usage: plot_frame_time.py filename")

# Open file
f = open(sys.argv[1], 'r')

times = []
# For each line
for line in f:
  # Use regular expressions to parse
  r = re.search("(\d+)us", line)
  if r:
    times.append(r.group(1))

# Plot
fig = plot.figure()
ax1 = plot.subplot(111)
ax1.plot(times)
ax1.set_xlabel("Frame")
ax1.set_ylabel("Execution time [us]")
plot.show()

# Close files
f.close()
