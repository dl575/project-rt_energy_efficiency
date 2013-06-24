#!/usr/bin/python -u

"""
Parse output of ffmpeg which includes frame timing and hex dump
of ist data structure. Dumps out to format for use with libsvm.

usage: process_ist.py tracefile 
"""

import re
import sys

"""
Break ist into a series of integers.
"""
def break_ist(ist):
  int_list = []
  for i in range(0, len(ist), 4):
    int_list.append(int(ist[i:i+4], 16))
  return int_list

# Parse inputs
if len(sys.argv) != 2:
  raise Exception(__doc__)
trace_filename = sys.argv[1]

tracefile = open(trace_filename, 'r')

# Find average frame time
total_frame_time = 0
total_frames = 0

trace = {}
# Start parsing traces
for line in tracefile:
  # Look for ist dump
  res = re.search("ist = ([0-9a-f]+)", line)
  if res:
    ist = res.group(1)

  # Look for frame line
  res = re.search("dlo: Frame ([0-9]+) = ([0-9]+)us", line)
  if res:
    frame_num = int(res.group(1))
    frame_time = int(res.group(2))

    total_frame_time += frame_time
    total_frames += 1

    if ist in trace:
      trace[ist].append(frame_time)
      print "Duplicate ist"
    else:
      trace[ist] = [frame_time]

tracefile.close()

# Classify based on average frame time
avg_frame_time = float(total_frame_time)/total_frames
# Print out for libsvm format
for (ist, [frame_time]) in trace.iteritems():
  if frame_time < avg_frame_time:
    print "-1 ",
  else:
    print "+1 ",
  ist_list = break_ist(ist)
  for (i, ist_chunk) in enumerate(ist_list):
    print "%d:%d " % (i, ist_chunk),
  print
