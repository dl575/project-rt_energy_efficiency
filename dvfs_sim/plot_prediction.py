#!/usr/bin/python

import os
import sys
from parse_lib import *
from pylab import *

colors = ["#fdbf6f", "#a6cee3", "#1f78b4", "#b2df8a", "#33a02c", "#fb9a99", "#e31a1c"]

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
plot(times, 'k', linewidth=2)

# Predicted times
predict_dir = "predict_times/"
#policies = ["policy_average", "policy_pid_timeliness", "policy_pid_energy", "policy_data_dependent", "policy_data_dependent2", "policy_data_dependent_oracle"]
policies = ["policy_pid_timeliness", "policy_data_dependent_oracle", "policy_data_dependent_lp"]
for (i, policy) in enumerate(policies):
  filename = "predict_times/%s-%s.txt" % (policy, benchmark)
  predict_times = read_predict_file(filename)
  plot(predict_times, '.-', color=colors[i], linewidth=2)
legend(["original"] + policies, loc="upper center", ncol=2)
ylim([0, 3*max(times)])

tight_layout()
show()
