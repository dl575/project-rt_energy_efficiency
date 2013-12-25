#!/usr/bin/python

"""
Parse output of ffmpeg which includes slice timing and a list
of auto-generated metrics.
Slice lines are in th form:
  dlo: Slice <slice number> = <time>us
Metrics lines are in the form:
  auto metrics = (<metric0>, <metric1>, ..., )

usage: ffmpeg_parse_auto_metrics.py infile svmfile [threshold]
"""

import sys
import re

def parse():
  slice_times = []
  metrics = []
  # Parse through file
  f = open(filename, 'r')
  for line in f:
    # Find metrics
    res = re.search("auto metrics = \((.*)\)", line)
    if res:
      # Remove any extra ',' and split into list
      res = res.group(1).strip().strip(',').split(',')
      # Convert to integers
      res = [int(x) for x in res]
      metrics.append(res)

    # Find slice times
    res = re.search("time ([0-9]+) = ([0-9]+) us", line)
    if res:
      slice_times.append(int(res.group(2)))

  f.close()

  return (slice_times, metrics)

"""
Identify indices of metrics which are not always constant.
"""
def find_non_constant_metrics(metrics):
  # Number of different metric types
  n_metrics = len(metrics[0])

  non_constant_metrics = []
  # For each metric type
  for n in range(n_metrics):
    # Check through each data point
    dref = metrics[0][n]
    for d in range(1, len(metrics)):
      # Different value, go to next metric type
      if metrics[d][n] != dref:
        non_constant_metrics.append(n)
        break

  """
  # Find constant metrics based on which ones were non-constant
  constant_metrics = []
  for n in range(n_metrics):
    if n not in non_constant_metrics:
      constant_metrics.append(n)
  """

  return non_constant_metrics

"""
metrics is a list of data vectors. ids is a list of indices.
filter_metrics returns a list of data vectors which only include the
indices indicated by ids.
"""
def filter_metrics(metrics, ids):
  filtered_metrics = []
  for m in metrics:
    filtered_metrics.append([m[i] for i in ids])
  return filtered_metrics

"""
Convert times and metrics into format suitable for libsvm.
"""
def output_svm(times, metrics, threshold=None):
  # Default threshold is average time
  if not threshold:
    threshold = float(sum(times))/len(times)

  output_str = ""

  for (t, m) in zip(times, metrics):
    if t < threshold:
      output_str += "-1 "
    else:
      output_str += "1 "

    output_str += ' '.join(["%d:%d " % (xi + 1, x) for (xi, x) in enumerate(m)])

    output_str += "\n"
  
  return output_str

"""
Output raw data in rows starting with time followed by metrics.
"""
def output_raw(times, metrics):
  output_str = ""
  for (t, m) in zip(times, metrics):
    output_str += ' '.join([str(t)] + [str(x) for x in m]) 
    output_str += "\n"
  return output_str

"""
Return the average of the values in list l.
"""
def average(l):
  return sum([float(x) for x in l])/len(l)

if __name__ == "__main__":

  # Handle script arguments
  if len(sys.argv) <= 2 or len(sys.argv) >= 5:
    print __doc__
    sys.exit()
  filename = sys.argv[1]
  svm_filename = sys.argv[2]
  if len(sys.argv) == 4:
    threshold = float(sys.argv[3])
  else:
    threshold = None

  # Parse times and metrics
  (slice_times, metrics) = parse()

  # Find metrics that are not always constant
  non_constant = find_non_constant_metrics(metrics)
  # Remove metrics that are always constant
  filtered = filter_metrics(metrics, non_constant)

  # Print out average time
  avg_slice_time = average(slice_times)
  print "Average frame time = %f" % (avg_slice_time)

  # Convert to SVM format
  output_str = output_svm(slice_times, filtered, threshold)
  # Write to file
  f = open(svm_filename, 'w')
  f.write(output_str)
  f.close()


  # # Output raw
  # output_str = output_raw(slice_times, filtered)
  # f = open(svm_filename, 'w')
  # f.write(output_str)
  # f.close()

