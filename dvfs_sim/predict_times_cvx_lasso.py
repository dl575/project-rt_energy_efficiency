#!/usr/bin/python

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

def run_prediction(train_filename, test_filename, policy, lasso_weight):
  """
  Runs [policy] on the data in [filename] in order to predict the execution
  time of tasks.  Returns the predicted times and the original observed
  execution times.
  """
  # Get base execution times
  train_times = parse_execution_times(train_filename)
  test_times = parse_execution_times(test_filename)
  # Loop counter metrics
  train_metrics = parse(train_filename, "loop counter [0-9 ]*= \((.*)\)")
  train_metrics = [x.strip().strip(',').split(',') for x in train_metrics]
  train_metrics = [[int(y) for y in x] for x in train_metrics]
  test_metrics = parse(test_filename, "loop counter [0-9 ]*= \((.*)\)")
  test_metrics = [x.strip().strip(',').split(',') for x in test_metrics]
  test_metrics = [[int(y) for y in x] for x in test_metrics]
  # Add function pointers
  fp = function_pointers(train_filename)
  if fp:
    train_metrics = map(lambda x, y: x+y, train_metrics, fp)
  fp = function_pointers(test_filename)
  if fp:
    test_metrics = map(lambda x, y: x+y, test_metrics, fp)

  # Predict times using the passed policy
  predict_times = policy(train_times=train_times, train_metrics=train_metrics, test_times=test_times, test_metrics=test_metrics, lasso_weight=lasso_weight)

  s = 0
  for i in range(1, len(predict_times)):
    s += abs(predict_times[i] - predict_times[i-1])

  return (predict_times, test_times)

input_data_dir = "data/"
output_dir = "predict_times/"
# Create output directory if it does not exist
if not os.path.isdir(output_dir):
  os.system("mkdir " + output_dir)
policies = [
    #policy_tuned_pid,
    #policy_data_dependent_oracle, 
    #policy_data_dependent_lp, 
    #policy_cvx_least_squares,
    policy_cvx_lasso,
    #policy_oracle,
    ]

# For each DVFS policy
#for policy in policies:
policy = policy_cvx_lasso
for lasso_weight in [0.001, 0.01, 0.1, 1, 10, 100]:
  print policy.__name__ + "_%f" % (lasso_weight)
  for benchmark in benchmarks:
    # Predict execution times
    train_filename = "%s/%s/%s0.txt" % (input_data_dir, benchmark, benchmark)
    test_filename = "%s/%s/%s1.txt" % (input_data_dir, benchmark, benchmark)
    print "  " + train_filename
    (predict_times, times) = run_prediction(train_filename, test_filename, policy, lasso_weight=lasso_weight)
    # Write prediction out to file
    out_file = open("%s/%s_%s-%s.txt" % (output_dir, policy.__name__, lasso_weight, benchmark), 'w')
    out_file.write("\n".join([str(x) for x in predict_times]))
    out_file.close()


