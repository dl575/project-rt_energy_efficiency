#!/usr/bin/python

import os
import sys
from parse_lib import *
from dvfs_sim_lib import *

def run(filename, policy):
  # Get base execution times
  times = parse_execution_times(filename)
  # Set deadline as worst-case observed time
  deadline = max(times)
  # Loop counter metrics
  metrics = parse(filename, "loop counter [0-9 ]*= \((.*)\)")
  metrics = [x.strip().strip(',').split(',') for x in metrics]
  metrics = [[int(y) for y in x] for x in metrics]

  # Predict times using the passed policy
  predict_times = policy(times=times, metrics=metrics)
  # Determine frequency to set DVFS to
  margin = 1.1
  frequencies = [scale_frequency_perfect(margin * t, deadline) for t in predict_times]
  #frequencies = [scale_frequency(margin * t, deadline) for t in predict_times]
  # Time with DVFS
  result_times = [dvfs_time(t, f) for (t, f) in zip(times, frequencies)]

  # Print out timeliness metrics
  #print deadline_misses(result_times, deadline), ",",
  #print normalized_tardiness(result_times, deadline)[0], ",",
  # Print out energy usage
  print energy(frequencies), ",",

for root, dirnames, filenames in os.walk("data/"):
  print list_to_csv([""] + filenames)

# For each DVFS policy
for policy in [policy_average, policy_pid_timeliness, policy_pid_energy, policy_data_dependent, policy_data_dependent2, policy_data_dependent_oracle]:
  print policy.__name__, ",",
  # For each data file
  for root, dirnames, filenames in os.walk("data/"):
    for filename in filenames:
      full_path = os.path.join(root, filename)
      run(full_path, policy)
  print

