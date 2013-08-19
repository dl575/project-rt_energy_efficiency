#!/usr/bin/python -u

"""
Parse output of ffmpeg which includes frame timing and hex dump
of ist data structure. Dumps out to format for use with libsvm.
Treshold in microseconds can be passed. If not, average frame 
time is used.

usage: process_ist.py tracefile svm_file [threshold]
"""

import re
import sys

# Parse inputs
if len(sys.argv) <= 2 or len(sys.argv) >= 5:
  raise Exception(__doc__)
trace_filename = sys.argv[1]
svm_filename = sys.argv[2]
if len(sys.argv) == 4:
  threshold = float(sys.argv[3])
else:
  # Threshold will be set to average later
  threshold = None

tracefile = open(trace_filename, 'r')

# Find average frame time
total_frame_time = 0
max_frame_time = 0
min_frame_time = sys.maxint
total_frames = 0

# trace elements are [frame_time, data1, data2, ...]
trace = []
time_index = 0
# Start parsing traces
for line in tracefile:
  res = re.search("dlo: metrics = \((.+)\)", line)
  if res:
    trace_vector = [float(x) for x in res.group(1).split(",")]
    trace.append([0] + trace_vector)

  # Look for execution time
  res = re.search("dlo: timing = ([0-9]+)", line)
  if res:
    frame_time = float(res.group(1))

    # Save information to calculate max/min/average
    total_frame_time += frame_time
    total_frames += 1
    if frame_time > max_frame_time:
      max_frame_time = frame_time
    if frame_time < min_frame_time:
      min_frame_time = frame_time

    if time_index >= len(trace):
      continue
    # Set frame time, metrics were set previously
    trace[time_index][0] = frame_time
    # Increase index for frame time
    time_index += 1

tracefile.close()


run_pca = False
if run_pca:
  # PCA Analysis
  import numpy
  from sklearn.decomposition import PCA
  # Create numpy array of metrics
  np_data = numpy.array([x[1:-1] for x in trace])
  # Perform PCA 
  pca = PCA(n_components=5)
  pca.fit(np_data)
  # print
  print "PCA vectors:"
  print pca.components_
  print
  print "PCA variance:"
  print pca.explained_variance_ratio_
  # Transform data
  pca_data = pca.transform(np_data)

# Write out libsvm file
svm_file = open(svm_filename, 'w')

# Calculate average frame time
avg_frame_time = float(total_frame_time)/total_frames
print "Frame time range = (%f, %f)" % (min_frame_time, max_frame_time)
print "Average frame time = %f" % (avg_frame_time)
# Default treshold is the average
if threshold == None:
  threshold = avg_frame_time

num_slow_frames = 0
num_fast_frames = 0
# Write out to file for libsvm format
for trace_item in trace:
  frame_time = trace_item[0]
  # 2 class
  if frame_time < threshold:
    svm_file.write("-1 ")
    num_slow_frames += 1
  else:
    svm_file.write("1 ")
    num_fast_frames += 1

  # Features
  for i in range(1, len(trace_item)):
    svm_file.write("%d:%d " % (i, trace_item[i]))

  # Use length of ist
  #svm_file.write("0:%d " % len(ist))
  # Break ist string into individual integers
  # ist_list = break_ist(ist)
  # for (i, ist_chunk) in enumerate(ist_list):
  #   svm_file.write("%d:%d " % (i + 1, ist_chunk))

  svm_file.write("\n")

svm_file.close()

# Print out percentage above and below threshold
# print "Slow frames = %f" % (float(num_slow_frames)/total_frames)
# print "Fast frames = %f" % (float(num_fast_frames)/total_frames)
