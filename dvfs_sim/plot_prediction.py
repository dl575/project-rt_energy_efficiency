#!/usr/bin/python

import os
import sys
import tsg_plot
from parse_lib import *
from pylab import *

#colors = ["#fdbf6f", "#a6cee3", "#1f78b4", "#b2df8a", "#33a02c", "#fb9a99", "#e31a1c"]
colors = ['#e41a1c', '#377eb8', '#4daf4a', '#984ea3', '#ff7f00', '#ffff33', '#a65628', '#f781bf', '#999999']

def plot(data, labels):
  opts = tsg_plot.PlotOptions()

  opt_dict = \
      {
          "data" : data,
          "labels" : labels,
          "plot_type" : "line",
          "figsize" : (7, 2.5),
          "fontsize" : 8,
          "file_name" : "plot_prediction.pdf",
          "symbols" : ['']*10,
          "legend_ncol" : 4,
          "xlabel" : "Job",
          "ylabel" : "Execution Time [us]",
          #"colors" : ["#000000", "#e31a1c"]
          "colors" : tsg_plot.colors["qualitative"],

          "linestyles" : ['-', '-', '--', '--'],
      }
  for k, v in opt_dict.iteritems():
    setattr(opts, k, v)
  tsg_plot.add_plot(opts)

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
data = []
labels = ["Baseline"]
times = parse_execution_times("data/%s/%s1.txt" % (benchmark, benchmark))
data.append(times)
#plot(times, 'k', linewidth=2)
#print ','.join(map(str, times))

# Predicted times
predict_dir = "predict_times/"
#policies = ["policy_tuned_pid", "policy_least_squares", "policy_conservative"]
policies = ["policy_oracle", "policy_cvx_conservative_lasso"]
labels += policies
for (i, policy) in enumerate(policies):
  filename = predict_dir + "%s-%s.txt" % (policy, benchmark)
  predict_times = read_predict_file(filename)
  data.append(predict_times)

plot(data, labels)

