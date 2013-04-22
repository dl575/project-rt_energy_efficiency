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

# Separately save times that are above or below the threshold
threshold = 41700
fast_times = {}
slow_times = {}
# For each line
for line in f:
  # Use regular expressions to parse
  r = re.search("Frame (\d+) = (\d+)us", line)
  if r:
    frame = int(r.group(1))
    time = int(r.group(2))
    if time > threshold:
      slow_times[frame] = time
    else:
      fast_times[frame] = time

# Plot
fig = plot.figure()
ax1 = plot.subplot(111)
ax1.scatter(fast_times.keys(), fast_times.values(), c='b')
ax1.scatter(slow_times.keys(), slow_times.values(), c='r')
ax1.set_xlabel("Frame")
ax1.set_ylabel("Execution time [us]")
plot.show()

# Close files
f.close()
