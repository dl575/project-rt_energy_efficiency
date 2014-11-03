#!/usr/bin/python

import os
import sys
from parse_lib import *
from dvfs_sim_lib import *

def run_prediction(filename, policy):
  """
  Runs [policy] on the data in [filename] in order to predict the execution
  time of tasks.  Returns the predicted times and the original observed
  execution times.
  """
  # Get base execution times
  times = parse_execution_times(filename)
  # Loop counter metrics
  metrics = parse(filename, "loop counter [0-9 ]*= \((.*)\)")
  metrics = [x.strip().strip(',').split(',') for x in metrics]
  metrics = [[int(y) for y in x] for x in metrics]

  # Predict times using the passed policy
  predict_times = policy(times=times, metrics=metrics)

  return (predict_times, times)

input_data_dir = "data/"
output_dir = "predict_times/"

# Print out benchmark filenames
"""
for root, dirnames, filenames in os.walk(input_data_dir):
  print list_to_csv([""] + filenames)
"""

# For each DVFS policy
#for policy in [policy_average, policy_pid_timeliness, policy_pid_energy, policy_data_dependent, policy_data_dependent2, policy_data_dependent_oracle, policy_oracle]:
for policy in [policy_pid_timeliness, policy_data_dependent2, policy_data_dependent_oracle, policy_data_dependent_lp, policy_oracle]:
  print policy.__name__
  # For each data file
  for root, dirnames, filenames in os.walk(input_data_dir):
    for filename in filenames:
      full_path = os.path.join(root, filename)
      # Predict execution times
      (predict_times, times) = run_prediction(full_path, policy)
      # Write prediction out to file
      out_file = open("%s/%s-%s" % (output_dir, policy.__name__, filename), 'w')
      out_file.write("\n".join([str(x) for x in predict_times]))
      out_file.close()

