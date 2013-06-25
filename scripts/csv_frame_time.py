#!/usr/bin/python

"""
Generate csv file of frame times.
"""

import re
import sys

if len(sys.argv) != 2:
  raise Exception("usage: csv_frame_time.py filename")

# Open file
f = open(sys.argv[1], 'r')

# Separately save times that are above or below the threshold
threshold = 41700
times = {}
# For each line
for line in f:
  # Use regular expressions to parse
  r = re.search("Frame (\d+) = (\d+)us", line)
  if r:
    frame = int(r.group(1))
    time = int(r.group(2))
    times[frame] = time

for (frame, time) in times.iteritems():
  print "%d, %d" % (frame, time)

# Close files
f.close()
