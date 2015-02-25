#!/usr/bin/python

import os
import sys
from parse_lib import *
from pylab import *

#colors = ["#fdbf6f", "#a6cee3", "#1f78b4", "#b2df8a", "#33a02c", "#fb9a99", "#e31a1c"]
colors = ['#e41a1c', '#377eb8', '#4daf4a', '#984ea3', '#ff7f00', '#ffff33', '#a65628', '#f781bf', '#999999']

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
times = parse_execution_times("data/%s/%s1.txt" % (benchmark, benchmark))
plot(times, 'k', linewidth=2)
#print ','.join(map(str, times))

# Predicted times
predict_dir = "predict_times/"
policies = ["policy_tuned_pid", "policy_data_dependent_oracle", "policy_data_dependent_lp"]
for (i, policy) in enumerate(policies):
  filename = predict_dir + "%s-%s.txt" % (policy, benchmark)
  predict_times = read_predict_file(filename)
  #print ','.join(map(str, predict_times))
  plot(predict_times, '.', color=colors[i], linewidth=1)
#legend(["original"] + policies, loc="upper center", ncol=2)
legend(["Execution Time", "PID", "Least-Squares Prediction", "Conservative Prediction"])
ylim([0, 3*max(times)])
#ylim([0, 1000])

tight_layout()
show()
