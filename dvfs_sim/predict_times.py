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

  # Separate test set
  if test_filename:
    # Get base execution times
    test_times = parse_execution_times(test_filename)
    # Loop counter metrics
    test_metrics = parse(test_filename, "loop counter [0-9 ]*= \((.*)\)")
    test_metrics = [x.strip().strip(',').split(',') for x in test_metrics]
    test_metrics = [[int(y) for y in x] for x in test_metrics]
    # Add function pointers
    fp = function_pointers(test_filename)
    if fp:
      test_metrics = map(lambda x, y: x+y, test_metrics, fp)
    # Predict times using the passed policy
    predict_times = policy(train_times=train_times, train_metrics=train_metrics, test_times=test_times, test_metrics=test_metrics)
  # Use training set for test
  else:
    test_times = parse_execution_times(train_filename)
    # Predict times using the passed policy
    predict_times = policy(train_times=train_times, train_metrics=train_metrics, test_times=None, test_metrics=None)

  s = 0
  for i in range(1, len(predict_times)):
    s += abs(predict_times[i] - predict_times[i-1])

  return (predict_times, test_times)

if __name__ == "__main__":

  no_test = False
  if len(sys.argv) > 1:
    flags = sys.argv[1:]
    # Don't use a test set (test on training set)
    if "--no_test" in flags:
      no_test = True
    # Specify a specific benchmark
    for flag in flags:
      if "--benchmark" in flag:
        benchmarks = [flag.split('=')[1]]

  input_data_dir = "data/"
  output_dir = "predict_times/"
  # Create output directory if it does not exist
  if not os.path.isdir(output_dir):
    os.system("mkdir " + output_dir)
  policies = [
      #policy_tuned_pid,
      policy_conservative,
      policy_cvx_conservative_lasso,
      #policy_oracle,
      ]

  # For each DVFS policy
  for policy in policies:
    print policy.__name__
    for benchmark in benchmarks:
      # Predict execution times
      train_filename = "%s/%s/%s0.txt" % (input_data_dir, benchmark, benchmark)
      test_filename = "%s/%s/%s1.txt" % (input_data_dir, benchmark, benchmark)
      print "  " + train_filename
      if no_test:
        (predict_times, times) = run_prediction(train_filename, None, policy)
      else:
        (predict_times, times) = run_prediction(train_filename, test_filename, policy)
      # Save lp solve output
      if policy == policy_conservative:
        os.system("cp temp.lps lps/%s.lps" % benchmark)
      elif policy == policy_least_squares:
        os.system("cp temp.lps regression_coeffs/%s.lps" % benchmark)
      elif policy == policy_cvx_conservative_lasso:
        os.system("cp temp.lps cvx/%s.lps" % benchmark)
      elif policy == policy_lasso:
        os.system("cp temp.lps lasso/%s.lps" % benchmark)
      elif policy == policy_tuned_pid:
        os.system("cp temp.lps pid/%s.lps" % benchmark)
      # Write prediction out to file
      out_file = open("%s/%s-%s.txt" % (output_dir, policy.__name__, benchmark), 'w')
      out_file.write("\n".join([str(x) for x in predict_times]))
      out_file.close()

