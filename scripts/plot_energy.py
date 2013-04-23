#!/usr/bin/python

"""
Plot the cumulative energy usage.
"""

import re
import sys
import matplotlib.pyplot as plot

"""
Read from the file with passed filename. Parses execution time for each frame
and returns this as a list.
"""
def read_execution_time(filename):
  # Open file
  f = open(filename, 'r')

  times = []
  # For each line
  for line in f:
    # Use regular expressions to parse
    r = re.search("Frame (\d+) = (\d+)us", line)
    if r:
      frame = int(r.group(1))
      time = int(r.group(2))
      times.append(time)

  # Close file
  f.close()

  return times

if len(sys.argv) != 3:
  raise Exception("usage: plot_frame_time.py filename1 filename2")

# Read in execution times from files
times1 = read_execution_time(sys.argv[1])
times2 = read_execution_time(sys.argv[2])

# Power usage of first and second CPUs
power1 = 2.5e-6 # W, Atom N270
power2 = 95e-6 # W, Core i5-2500K

# Calculate cumulative energy usage
energy1 = [0]
for time in times1:
  energy1.append(time * power1 + energy1[-1])
energy2 = [0]
for time in times2:
  energy2.append(time * power2 + energy2[-1])

# Find limits of data for setting graph scales
xmin = 0
xmax = max(len(energy1), len(energy2))
ymin = 0
ymax = max(energy1 + energy2)

# Response time limit
threshold = 41700
energy_best = [0]
for (ti, time) in enumerate(times1):
  # If we go above the response time limit
  if time > threshold:
    # Use the faster CPU
    energy_best.append(times2[ti] * power2 + energy_best[-1])
  # Within response time limit
  else:
    # Can use the slower CPU
    energy_best.append(time * power1 + energy_best[-1])


# Plot
fig = plot.figure()

ax1 = plot.subplot(221)
ax1.scatter([i for i in range(len(energy1))], energy1, c='b', marker='.')
ax1.set_xlabel("Frame")
ax1.set_ylabel("Energy [J]")
ax1.set_xlim([xmin, xmax])
ax1.set_ylim([ymin, ymax])
ax1.set_title("Atom")

ax2 = plot.subplot(222)
ax2.scatter([i for i in range(len(energy2))], energy2, c='b', marker='.')
ax2.set_xlabel("Frame")
ax2.set_xlim([xmin, xmax])
ax2.set_ylim([ymin, ymax])
ax2.set_title("i5")

ax3 = plot.subplot(223)
ax3.scatter([i for i in range(len(energy_best))], energy_best, c='b', marker='.')
ax3.set_xlabel("Frame")
ax3.set_xlim([xmin, xmax])
ax3.set_ylim([ymin, ymax])
ax3.set_title("Hybrid")

plot.show()
