#!/usr/bin/python -u

"""
Parse output of ffmpeg which includes frame timing and hex dump
of ist data structure. Dumps out to format for use with libsvm.
Threshold in microseconds can be passed. If not, average frame 
time is used.

usage: process_ist.py tracefile svm_file [threshold]
"""

import re
import sys

# Parse inputs
if len(sys.argv) <= 1 or len(sys.argv) >= 5:
  raise Exception(__doc__)
trace_filename = sys.argv[1]
svm_filename = sys.argv[2]
if len(sys.argv) == 4:
  threshold = float(sys.argv[3])/1000000
else:
  # Threshold will be set to average later
  threshold = None

tracefile = open(trace_filename, 'r')

# Find average frame time
total_frame_time = 0
max_frame_time = 0
min_frame_time = sys.maxint
total_frames = 0

num_metrics = 5
# First data item is frame time followed by metrics
data = [list() for n in range(num_metrics + 1)]
# Start parsing traces
for line in tracefile:
  # # Look for feature data
  res = re.search("height/width = \(([0-9]+), ([0-9]+)\)", line)
  if res:
    data[1].append(int(res.group(1)) * int(res.group(2)))
  res = re.search("Packet size = ([0-9]+)", line)
  if res:
    data[2].append(int(res.group(1)))
  res = re.search("slice type = ([0-9]+)", line)
  if res:
    data[3].append(-1)
    data[4].append(-1)
    data[5].append(-1)
    slice_type = int(res.group(1))
    if slice_type == 1:
      data[3][-1] = 1
    elif slice_type == 2:
      data[4][-1] = 1
    elif slice_type == 3:
      data[5][-1] = 1

  # Look for frame line
  res1 = re.search("Frame ([0-9]+) time = ([0-9\.]+)", line)
  res2 = re.search("dlo: Frame ([0-9]+) = ([0-9]+)us", line)
  if res1:
    frame_num = int(res1.group(1))
    frame_time = float(res1.group(2))
  if res2:
    frame_num = int(res2.group(1))
    frame_time = float(res2.group(2))/1000000
  if res1 or res2:
    # Skip first frame
    if frame_num == 1:
      continue

    # Save information to calculate max/min/average
    total_frame_time += frame_time
    total_frames += 1
    if frame_time > max_frame_time:
      max_frame_time = frame_time
    if frame_time < min_frame_time:
      min_frame_time = frame_time

    data[0].append(frame_time)

tracefile.close()

# Write out libsvm file
svm_file = open(svm_filename, 'w')

# Calculate average frame time
avg_frame_time = float(total_frame_time)/total_frames
print "Frame time range = (%f, %f)" % (min_frame_time, max_frame_time)
print "Average frame time = %f" % avg_frame_time
# Default treshold is the average
if threshold == None:
  threshold = avg_frame_time

num_slow_frames = 0
num_fast_frames = 0
# Write out to file for libsvm format
for i in range(len(data[0])):
  frame_time = data[0][i]
  # 2 class
  if frame_time < threshold:
    svm_file.write("-1 ")
    num_slow_frames += 1
  else:
    svm_file.write("+1 ")
    num_fast_frames += 1

  # Features
  #for j in range(1, num_metrics + 1):
  #  svm_file.write("%d:%d " % (j, data[j][i]))
  svm_file.write("1:%d " % (data[2][i]))

  svm_file.write("\n")

svm_file.close()

# Print out percentage above and below threshold
# print "Slow frames = %f" % (float(num_slow_frames)/total_frames)
# print "Fast frames = %f" % (float(num_fast_frames)/total_frames)
