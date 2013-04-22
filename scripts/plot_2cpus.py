#!/usr/bin/python

"""
Plot the execution time for each frame.
"""

import re
import sys
import matplotlib.pyplot as plot

"""
Read from the file with passed filename. Parses execution time for each frame
and separates them into two dicts: slow_times and fast_times. These dicts are
indexed by frame number and store execution time.
"""
def read_execution_time(filename, threshold):
  # Open files
  f = open(filename, 'r')

  # Separately save times that are above or below the threshold
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

  # Close files
  f.close()

  return (slow_times, fast_times)

if len(sys.argv) != 3:
  raise Exception("usage: plot_frame_time.py filename1 filename2")

threshold = 41700
(slow1, fast1) = read_execution_time(sys.argv[1], threshold)
times1 = slow1 + fast1
(slow2, fast2) = read_execution_time(sys.argv[2], threshold)
times2 = slow2 + fast2

# Power usage of first and second CPUs
power1 = 2.5 # W, Atom N270
power2 = 95 # W, Core i5-2500K

# Find limits of data for setting graph scales
xmin = 0
xmax = max(len(times1), len(times2))
ymin = 0
ymax = max(times1.values() + times2.values())

# Plot
"""
fig = plot.figure()

ax1 = plot.subplot(121)
ax1.scatter(fast1.keys(), fast1.values(), c='b', marker='.')
ax1.scatter(slow1.keys(), slow1.values(), c='r', marker='.')
ax1.set_xlabel("Frame")
ax1.set_ylabel("Execution time [us]")
ax1.set_xlim([xmin, xmax])
ax1.set_ylim([ymin, ymax])
ax1.set_title("Atom")

ax2 = plot.subplot(122)
ax2.scatter(fast2.keys(), fast2.values(), c='b', marker='.')
ax2.scatter(slow2.keys(), slow2.values(), c='r', marker='.')
ax2.set_xlabel("Frame")
ax2.set_xlim([0, len(fast2) + len(slow2)])
ax2.set_xlim([xmin, xmax])
ax2.set_ylim([ymin, ymax])
ax2.set_title("i5")

plot.show()
"""
