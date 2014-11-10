#!/usr/bin/python

from parse_lib import *
from dvfs_sim_lib import *

def run_dvfs(predict_times, times, deadline=None, dvfs_levels=None, margin=1.1):
  """
  Given predicted times and original execution times, sets DVFS level. The max
  original execution time is used as the deadline by default. [dvfs_levels] is
  a list of allowed normalized frequencies. If no list is specified, then a
  continuous range of dvfs_levels is allowed. Prediction times are multiplied
  by [margin] to allow a conservative margin to be added. Returns the resulting
  execution times, the frequencies selected, and the deadline used.
  """
  # Set deadline as worst-case observed time
  if not deadline:
    deadline = max(times)
  # Determine frequency to set DVFS to
  if dvfs_levels:
    frequencies = [scale_frequency(margin * t, deadline, dvfs_levels=dvfs_levels) for t in predict_times]
  else:
    frequencies = [scale_frequency_perfect(margin * t, deadline) for t in predict_times]
  # Time with DVFS
  result_times = [dvfs_time(t, f) for (t, f) in zip(times, frequencies)]

  return (result_times, frequencies, deadline)

def read_predict_file(filename):
  f = open(filename, 'r')
  data = []
  for line in f:
    data.append(float(line))
  f.close()
  return data


#policies = ["policy_average", "policy_pid_timeliness", "policy_pid_energy", "policy_data_dependent", "policy_data_dependent2", "policy_data_dependent_oracle", "policy_pid_energy_plus3s", "policy_data_dependent_plus3s", "policy_data_dependent_plus3s_oracle", "policy_oracle"]
# Only non-data-dependent policies
#policies = ["policy_average", "policy_pid_timeliness", "policy_pid_energy", "policy_oracle"]
policies = ["policy_pid_timeliness", "policy_data_dependent2", "policy_data_dependent_oracle", "policy_data_dependent_lp", "policy_oracle"]
benchmarks = ["sha", "rijndael", "stringsearch", "xpilot", "julius", "freeciv"]

for metric in [deadline_misses, avg_normalized_tardiness, energy]:
  print metric.__name__
  print list_to_csv([""] + benchmarks + ["average"])
  for policy in policies:
    print policy, ",", 
    sum_metric = 0
    for benchmark in benchmarks:
      # Read in execution times and predicted times
      times = parse_execution_times("data/%s.txt" % (benchmark))
      predict_times = read_predict_file("predict_times/%s-%s.txt" % (policy, benchmark))

      # Perform DVFS
      #(result_times, frequencies, deadline) = run_dvfs(predict_times, times, dvfs_levels=default_dvfs_levels, deadline=None) # Discrete
      (result_times, frequencies, deadline) = run_dvfs(predict_times, times, dvfs_levels=None, deadline=None) # Continuous

      # Calculate metric of interest
      metric_result = metric(result_times, frequencies, deadline)
      sum_metric += metric_result
      print metric_result, ",",
    # Output average
    print sum_metric/len(benchmarks)
  print
