#!/usr/bin/python

import os
import sys
from parse_lib import *
from dvfs_sim_lib import *
from pylab import *

colors = ["#fdbf6f", "#a6cee3", "#1f78b4", "#b2df8a", "#33a02c", "#fb9a99", "#e31a1c"]

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

if len(sys.argv) == 2:
  benchmark = sys.argv[1]
else:
  print "usage: plot_prediction.py benchmark"
  sys.exit()

# Original execution time
times = parse_execution_times("data/%s.txt" % (benchmark))

# Predicted times
predict_dir = "predict_times/"
policies = ["policy_average", "policy_pid_timeliness", "policy_pid_energy", "policy_data_dependent", "policy_data_dependent2", "policy_data_dependent_oracle"]
for (i, policy) in enumerate(policies):
  filename = "predict_times/%s-%s.txt" % (policy, benchmark)
  predict_times = read_predict_file(filename)
  #(result_times, frequencies, deadline) = run_dvfs(predict_times, times, dvfs_levels=default_dvfs_levels) 
  #(result_times, frequencies, deadline) = run_dvfs(predict_times, times, dvfs_levels=[0.05, 0.1, 0.15, 0.2, 0.25, 0.5, 0.75, 1.0]) 
  (result_times, frequencies, deadline) = run_dvfs(predict_times, times, dvfs_levels=None) 

  subplot(2, 1, 1)
  plot(result_times, '.-', color = colors[i], linewidth=2)
  subplot(2, 1, 2)
  plot(frequencies, '.-', color=colors[i])

subplot(2, 1, 1)
plot([max(times)]*len(times), 'k--', linewidth=3)
plot(times, 'k')
ylim([0, 3*max(times)])

legend(policies, loc="upper center", ncol=2)


tight_layout()
show()
