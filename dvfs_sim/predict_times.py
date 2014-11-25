#!/usr/bin/python

import os
import sys
from parse_lib import *
from dvfs_sim_lib import *

def run_prediction(train_filename, test_filename, policy):
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

  # Predict times using the passed policy
  predict_times = policy(train_times=train_times, train_metrics=train_metrics, test_times=test_times, test_metrics=test_metrics)

  s = 0
  for i in range(1, len(predict_times)):
    s += abs(predict_times[i] - predict_times[i-1])
  print float(s)/(len(predict_times) - 1)
  import numpy
  print numpy.std(predict_times)
  print min(predict_times), max(predict_times)


  return (predict_times, test_times)

input_data_dir = "data/"
output_dir = "predict_times/"
# Create output directory if it does not exist
if not os.path.isdir(output_dir):
  os.system("mkdir " + output_dir)
policies = [
    policy_tuned_pid,
    #policy_data_dependent_oracle, 
    #policy_data_dependent_lp, 
    #policy_oracle
    ]

# For each DVFS policy
for policy in policies:
  print policy.__name__
  for benchmark in benchmarks:
    # Predict execution times
    train_filename = "%s/%s/%s0.txt" % (input_data_dir, benchmark, benchmark)
    test_filename = "%s/%s/%s1.txt" % (input_data_dir, benchmark, benchmark)
    print "  " + train_filename
    (predict_times, times) = run_prediction(train_filename, test_filename, policy)
    # Save lp solve output
    if policy == policy_data_dependent_lp:
      os.system("cp temp.lps lps/%s.lps" % benchmark)
    # Write prediction out to file
    out_file = open("%s/%s-%s.txt" % (output_dir, policy.__name__, benchmark), 'w')
    out_file.write("\n".join([str(x) for x in predict_times]))
    out_file.close()


