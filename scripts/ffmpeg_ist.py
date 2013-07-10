#!/usr/bin/python -u

"""
Parse output of ffmpeg which includes frame timing and hex dump
of ist data structure. Dumps out to format for use with libsvm.
Treshold can be passed. If not, average frame time is used.

usage: process_ist.py tracefile [threshold]
"""

import re
import sys

"""
Break ist into a series of integers.
"""
def break_ist(ist):
  int_list = []
  chunk_len = 4
  for i in range(0, len(ist), chunk_len):
    int_list.append(int(ist[i:i+chunk_len], 16))
  return int_list

# Parse inputs
if len(sys.argv) <= 1 or len(sys.argv) >= 5:
  raise Exception(__doc__)
trace_filename = sys.argv[1]
svm_filename = sys.argv[2]
if len(sys.argv) == 4:
  threshold = int(sys.argv[3])
else:
  # Threshold will be set to average later
  threshold = None

tracefile = open(trace_filename, 'r')

# Find average frame time
total_frame_time = 0
max_frame_time = 0
min_frame_time = sys.maxint
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
    if frame_time > max_frame_time:
      max_frame_time = frame_time
    if frame_time < min_frame_time:
      min_frame_time = frame_time

    if ist in trace:
      trace[ist].append(frame_time)
      print "Duplicate ist"
    else:
      trace[ist] = [frame_time]

tracefile.close()

# Classify based on average frame time
avg_frame_time = float(total_frame_time)/total_frames
print avg_frame_time
quart50 = avg_frame_time
quart75 = (avg_frame_time + max_frame_time)/2
quart25 = (avg_frame_time + min_frame_time)/2

svm_file = open(svm_filename, 'w')
# Print out for libsvm format
for (ist, [frame_time]) in trace.iteritems():
  # 2 class
  if threshold == None:
    threshold = avg_frame_time
  if frame_time < threshold:
    svm_file.write("-1 ")
  else:
    svm_file.write("1 ")
  # 4 class
  """
  if frame_time < quart25:
    print "0 ",
  elif frame_time < quart50:
    print "1 ",
  elif frame_time < quart75:
    print "2 ",
  else:
    print "3 ",
  """
  # Print execution time for regression
  """
  print "%d " % frame_time,
  """
  # Break ist string into individual integers
  # ist_list = break_ist(ist)
  # for (i, ist_chunk) in enumerate(ist_list):
  #   svm_file.write("%d:%d " % (i + 1, ist_chunk))
  svm_file.write("1:%d" % len(ist))
  svm_file.write("\n")

