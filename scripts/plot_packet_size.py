#!/usr/bin/python -u

import re
import sys
import matplotlib.pyplot as plot

# Parse inputs
if len(sys.argv) != 2:
  raise Exception(__doc__)
trace_filename = sys.argv[1]

tracefile = open(trace_filename, 'r')

num_metrics = 6
metrics = [0]*num_metrics
trace = []
# Start parsing traces
for line in tracefile:
  res = re.search("height/width = \(([0-9]+), ([0-9]+)\)", line)
  if res:
    metrics[0] = int(res.group(1)) * int(res.group(2))
  res = re.search("Packet size = ([0-9]+)", line)
  if res:
    metrics[1] = int(res.group(1))
  res = re.search("slice type = ([0-9]+)", line)
  if res:
    slice_type = int(res.group(1))
    if slice_type == 1:
      metrics[2:5] = [ 1, -1, -1]
    elif slice_type == 2:
      metrics[2:5] = [-1,  1, -1]
    elif slice_type == 3:
      metrics[2:5] = [-1, -1,  1]
    else:
      metrics[2:5] = [-1, -1, -1]

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

    #trace.append([frame_time, height, width, packet_size])
    trace.append([frame_time] + metrics)

tracefile.close()

# Plot packet size and frame time
fig = plot.figure()
ax1 = plot.subplot(111)

frame_times = [t[0] for t in trace]
frame_times = [f/max(frame_times) for f in frame_times]
ax1.plot([x for x in range(len(trace))], frame_times, 'b')
packet_sizes = [t[2] for t in trace]
packet_sizes = [float(p)/max(packet_sizes) for p in packet_sizes]
ax1.plot([x for x in range(len(trace))], packet_sizes, 'r')

plot.show()


