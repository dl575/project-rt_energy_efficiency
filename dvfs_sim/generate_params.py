#!/usr/bin/env python

import os
import sys
from parse_lib import *
from dvfs_sim_lib import *

def function_pointers(filename):
  """ 
  Get function pointers from log file and convert them to 1-hot encoding.
  """
  # Get the function pointer lines
  func_ptrs = parse(filename, "function pointer = \((.*)\)")
  func_ptrs = [x.strip().strip(',').split(',') for x in func_ptrs]
  func_ptrs = [map(int, x) for x in func_ptrs]

  # No function pointers, return
  if len(func_ptrs) == 0:
    return []

  # Convert to 1-hot encoding
  unique_func_ptrs = list(set(reduce(lambda x, y: x+y, func_ptrs)))
  vectors = []
  for func_ptr in func_ptrs:
    vector = [0]*len(unique_func_ptrs) 
    for f in func_ptr:
      vector[unique_func_ptrs.index(f)] = 1 
    vectors.append(vector)
  return vectors

def run_prediction(train_filename, test_filename, policy):
  """
  Runs [policy] on the data in [filename] in order to predict the execution
  time of tasks.  Returns the predicted times and the original observed
  execution times.
  """
  # Get base execution times
  train_times = parse_execution_times(train_filename)
  # Loop counter metrics
  train_metrics = parse(train_filename, "loop counter [0-9 ]*= \((.*)\)")
  train_metrics = [x.strip().strip(',').split(',') for x in train_metrics]
  train_metrics = [[int(y) for y in x] for x in train_metrics]

  # Add function pointers
  fp = function_pointers(train_filename)
  if fp:
    train_metrics = map(lambda x, y: x+y, train_metrics, fp)
  train_metrics = [[1] + x for x in train_metrics]

  # Print train_metrics : Matrix X
  for j in xrange(0, len(train_metrics[0])):
    for i in xrange(0, len(train_metrics)):
      print "  params.xx[%d] = %f;" % (i+j*len(train_metrics), train_metrics[i][j])


  # Print time_metrics : Vector Y
  for i in xrange(0, len(train_times)):
    print "  params.yy[%d] = %f;" % (i, train_times[i])


if __name__ == "__main__":

  no_test = True
  input_data_dir = "data/"
  policies = [
#  policy_least_squares,
      policy_cvx_conservative_lasso,
      ]

  # For each DVFS policy
  for policy in policies:
    print policy.__name__
    for benchmark in benchmarks:
      # Predict execution times
      train_filename = "%s/%s/%s0.txt" % (input_data_dir, benchmark, benchmark)
      test_filename = "%s/%s/%s1.txt" % (input_data_dir, benchmark, benchmark)
      print "  " + train_filename
      run_prediction(train_filename, None, policy)
     
