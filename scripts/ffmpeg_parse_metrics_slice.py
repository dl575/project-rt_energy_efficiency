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

num_metrics = 4
metrics = [0]*num_metrics
past_times = [0]*10
# trace elements are [frame_time, data1, data2, ...]
trace = []
# Start parsing traces
for line in tracefile:
  # Look for feature data
  # res = re.search("metrics = ([0-9]+), ([0-9]+), ([01]), ([01]), ([01])", line)
  # if res:
  #   #metrics = [int(res.group(1)), int(res.group(2)), int(res.group(3)), int(res.group(4)), int(res.group(5))]
  #   metrics = [int(res.group(1)), int(res.group(2))]
  #   metrics.append(1 if int(res.group(3)) else -1)
  #   metrics.append(1 if int(res.group(4)) else -1)
  #   metrics.append(1 if int(res.group(5)) else -1)
  #res = re.search("height/width = \(([0-9]+), ([0-9]+)\)", line)
  #if res:
  #  metrics[0] = int(res.group(1)) * int(res.group(2))
  res = re.search("Packet size = ([0-9]+)", line)
  if res:
    metrics[0] = int(res.group(1))
  res = re.search("slice type = ([0-9]+)", line)
  if res:
    slice_type = int(res.group(1))
    if slice_type == 1:
      metrics[1:4] = [ 1, -1, -1]
    elif slice_type == 2:
      metrics[1:4] = [-1,  1, -1]
    elif slice_type == 3:
      metrics[1:4] = [-1, -1,  1]
    else:
      raise Exception("Unknown slice type = %d" % slice_type)

  # Look for frame line
  res1 = re.search("Slice ([0-9]+) time = ([0-9\.]+)", line)
  res2 = re.search("dlo: Slice ([0-9]+) = ([0-9]+)us", line)
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

    trace.append([frame_time] + metrics + past_times)
    #trace.append([frame_time] + past_times)
    #trace.append([frame_time] + metrics)

    # Save execution for use in future predictions
    past_times.append(frame_time*1000000)
    past_times.pop(0)

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
print "Frame time range = (%f, %f)" % (min_frame_time*1000000, max_frame_time*1000000)
print "Average frame time = %f" % (avg_frame_time*1000000)
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
